/*
 * Copyright (c) 2022 Braingines SA - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#include "FirProcessor.h"

#include "device/Properties.h"
#include "device/SM_FFT_parameters.cuh"
#include "ImpulseResponseStore.h"

#include <processor_api/PortChangedFlags.h>
#include <processor_api/PortDescription.h>
#include <processor_api/ProcessorSpecification.h>
#include <processor_api/MemoryManager.h>

#include <algorithm>
#include <map>

using namespace GPUA::processor::v2;

namespace {
template <class T, class U>
constexpr T divup(T a, U b) {
    return (a + b - 1) / b;
}

uint32_t getSampleBytes(PortDataType const& pdt) {
    if (pdt == PortDataType::eSample16) {
        return 2u;
    }
    if (pdt == PortDataType::eSample32) {
        return 4u;
    }
    if (pdt == PortDataType::eSample64) {
        return 8u;
    }

    throw std::runtime_error("Error getSampleBytes: unsupported sample type\n");
}
} // namespace

Module& FirProcessor::GetModule() const noexcept {
    return m_module;
}

ErrorCode FirProcessor::SetData(void* data, uint32_t data_size) noexcept {
    // make sure we get valid data
    if (data != nullptr && data_size == sizeof(FirConfig::Parameters)) {
        auto params = reinterpret_cast<const FirConfig::Parameters*>(data);
        // determine the message type - the processor only supports a fir message
        if (params->ThisMessage == FirConfig::Parameters::FirMessage) {
            if (m_old_choice != params->ir_index) {
                UpdateProcessorFilter(params->ir_index);
                m_old_choice = params->ir_index;
            }
            return ErrorCode::eSuccess;
        }
    }
    return ErrorCode::eFail;
}

ErrorCode FirProcessor::GetData(void* data, uint32_t& data_size) const noexcept {
    // not implemented; could potentially satisfy user queries
    return ErrorCode::eFail;
}

uint32_t FirProcessor::GetInputPortCount() const noexcept {
    // the fir_processor has one port (for the input data)
    return 1u;
}

ErrorCode FirProcessor::GetInputPort(uint32_t index, InputPort*& port) noexcept {
    // return the requested port
    if (index == 0) {
        port = this;
        return ErrorCode::eSuccess;
    }
    // or nullptr if the index is out-of-bounds (!= 0 in this case)
    port = nullptr;
    return ErrorCode::eOutOfRange;
}

ErrorCode FirProcessor::PrepareForProcess(const LaunchData& data, uint32_t expected_chunks) noexcept {
    // process the provided user-data
    SetData(data.app_data, data.app_data_size);

    // communicate a blueprint rebuild if anything changed that requires one
    if (m_changed)
        return ErrorCode::eBlueprintUpdateNeeded;

    return ErrorCode::eNoChangesNeeded;
}

ErrorCode FirProcessor::OnBlueprintRebuild(const ProcessorBlueprint*& blueprint) noexcept {
    m_changed = false;
    blueprint = &m_proc_data;
    return ErrorCode::eSuccess;
}

ErrorCode FirProcessor::PrepareChunk(void* proc_data, void** task_data, uint32_t chunk_id) noexcept {
    const auto& output_port = m_output_port->GetPortInfo();

    fir::ProcessorParameter processor_parameter_struct {};

    processor_parameter_struct.fourier_input_segments = reinterpret_cast<float2*>(m_fourier_input_segments->GetGpuPointer());
    processor_parameter_struct.overlap = reinterpret_cast<float*>(m_overlap->GetGpuPointer());

    int channel = 0;
#ifdef SHARED_IRS
    for (; channel < m_channel_count; ++channel) {
        processor_parameter_struct.fourier_impulse_response_segments[channel] =
            reinterpret_cast<float2*>(m_current_ir_filter->getSegments(channel, m_fourier_impulse_response_segments_length));
        processor_parameter_struct.real_filter[channel] =
            reinterpret_cast<float*>(m_current_ir_filter->getRawIR(channel));
    }
#else
    for (; channel < m_current_ir_filter->GetChannelCount(); ++channel) {
        processor_parameter_struct.fourier_impulse_response_segments[channel] =
            reinterpret_cast<float2*>(m_fourier_impulse_response_segments[channel]->GetGpuPointer());
        processor_parameter_struct.real_filter[channel] =
            reinterpret_cast<float*>(m_real_filter[channel]->GetGpuPointer());
    }
    for (; channel < m_channel_count; ++channel) {
        processor_parameter_struct.fourier_impulse_response_segments[channel] =
            reinterpret_cast<float2*>(m_fourier_impulse_response_segments[0]->GetGpuPointer());
        processor_parameter_struct.real_filter[channel] =
            reinterpret_cast<float*>(m_real_filter[0]->GetGpuPointer());
    }
#endif
    for (; channel < MAX_CHANNELS; ++channel) {
        processor_parameter_struct.fourier_impulse_response_segments[channel] = 0;
        processor_parameter_struct.real_filter[channel] = 0;
    }

    processor_parameter_struct.segments_count = static_cast<int>(m_segment_count);
    processor_parameter_struct.input_samples_per_iteration = static_cast<int>(m_input_size_per_iteration);
    processor_parameter_struct.fir_samples_per_iteration = static_cast<int>(m_fir_samples_per_segment);
    processor_parameter_struct.overlap_length = static_cast<int>(m_max_overlap);
    processor_parameter_struct.input_length = static_cast<int>(output_port.size_in_bytes / getSampleBytes(output_port.data_type));
    processor_parameter_struct.grain = static_cast<int>(m_real_grain);

    if (m_recompute_filter) {
        uint32_t offset = 0;
        if (m_real_grain < FftParameters::config::fft_length_quarter) {
            offset = m_real_grain * static_cast<uint32_t>(rand()) % m_input_size_per_iteration;
        }
        processor_parameter_struct.filter_length_to_translate_init = static_cast<int>(m_current_ir_filter->GetFilterLength());
        processor_parameter_struct.init_buffer_offset = static_cast<int>(offset);
        m_recompute_filter = false;
    }
    else {
        processor_parameter_struct.filter_length_to_translate_init = 0;
        processor_parameter_struct.init_buffer_offset = 0;
    }

    *reinterpret_cast<fir::ProcessorParameter*>(proc_data) = processor_parameter_struct;
    return ErrorCode::eSuccess;
}

void FirProcessor::OnProcessingEnd(bool after_fat_transfer) noexcept {
}

PortId FirProcessor::GetPortId() noexcept {
    return m_output_port->GetPortId();
}

ErrorCode FirProcessor::Connect(const OutputPort& data_port) noexcept {
    auto& input_port = data_port.GetPortInfo();

    if (input_port.type != PortType::eRegularPort ||
        input_port.data_type != PortDataType::eSample32 ||
        input_port.channel_count > MAX_CHANNELS) {
        return ErrorCode::eUnsupported;
    }

    auto& output_port = m_output_port->GetPortInfo();
    output_port = input_port;
    UpdateFilterCoefficients();
    output_port.grain = m_real_grain;
    output_port.transfer_to_cpu = false;
    output_port.is_produced = true;
    m_output_port->Changed(PortChangedFlags::eReset);

    size_t num_calls = divup(input_port.capacity_in_bytes / getSampleBytes(input_port.data_type), m_real_grain);
    if ((num_calls != m_proc_data.num_calls) || (m_gpu_task.block_count != m_channel_count)) {
        m_proc_data.num_calls = num_calls;
        m_gpu_task.block_count = m_channel_count;
        m_changed = true;
    }

    return ErrorCode::eSuccess;
}

ErrorCode FirProcessor::Disconnect() noexcept {
    auto& output_port = m_output_port->GetPortInfo();
    output_port.size_in_bytes = 0;
    output_port.grain = m_real_grain;
    output_port.offset = 0;
    output_port.oversampling_ratio = 0;
    output_port.is_produced = false;
    output_port.transfer_to_cpu = false;
    m_output_port->Changed(PortChangedFlags::eReset);

    return ErrorCode::eSuccess;
}

ErrorCode FirProcessor::InputPortUpdated(PortChangedFlags flags, const OutputPort& data_port) noexcept {
    auto& input_port = data_port.GetPortInfo();

    if (input_port.type != PortType::eRegularPort ||
        input_port.data_type != PortDataType::eSample32 ||
        input_port.channel_count > MAX_CHANNELS) {
        Disconnect();
        return ErrorCode::eUnsupported;
    }

    PortChangedFlags new_flags = flags & (~(PortChangedFlags::eGrainChanged | PortChangedFlags::eTransferToCpuChanged | PortChangedFlags::eProduceInfoChanged));
    if (static_cast<uint32_t>(new_flags) == 0) {
        return ErrorCode::eSuccess;
    }

    constexpr PortChangedFlags need_rebuild_flags = PortChangedFlags::eCapacityChanged | PortChangedFlags::eChannelCountChanged;
    if (static_cast<uint32_t>(flags & need_rebuild_flags) != 0) {
        m_changed = true;
    }

    auto& output_port = m_output_port->GetPortInfo();
    output_port = input_port;
    UpdateFilterCoefficients();
    output_port.grain = m_real_grain;
    output_port.transfer_to_cpu = false;
    output_port.is_produced = true;

    size_t num_calls = divup(input_port.capacity_in_bytes / getSampleBytes(input_port.data_type), m_real_grain);
    if ((num_calls != m_proc_data.num_calls) || (m_gpu_task.block_count != m_channel_count)) {
        m_proc_data.num_calls = num_calls;
        m_gpu_task.block_count = m_channel_count;
        m_changed = true;
    }

    if (flags == PortChangedFlags::eReset || flags == PortChangedFlags::eTypeChanged) {
        m_output_port->Changed(PortChangedFlags::eReset);
    }
    else {
        m_output_port->Changed(new_flags);
    }

    return ErrorCode::eSuccess;
}

uint32_t FirProcessor::GetInputGrain() const noexcept {
    return m_real_grain;
}

ErrorCode FirProcessor::GetPortDescription(const PortDescription*& description) const noexcept {
    static PortDescription desc {L"Input Port", L"Port forwarded to the output"};
    description = &desc;
    return ErrorCode::eSuccess;
}

ProcessorProfiler* FirProcessor::GetProcessorProfiler() noexcept {
    return this;
}

uint32_t FirProcessor::RunProfiling(const ProfileSpecification& spec, LatencyProfiler& profiler) noexcept {
    static size_t estimate = 100;
    if (estimate != 0)
        return estimate;

    PrepareChunk(spec.proc_param_buf, spec.task_param_bufs, 0);
    size_t testimate = profiler.RunProfiling(1, 10, m_gpu_task.thread_count);

    PrepareChunk(spec.proc_param_buf, spec.task_param_bufs, 0);
    size_t estimate_min = testimate;
    size_t estimate_max = 0;
    double estimate_avg = 0;
    size_t count = 100;
    for (size_t i = 0; i < count; ++i) {
        size_t testimate = profiler.RunProfiling(1, 10, m_gpu_task.thread_count);
        estimate_min = std::min(estimate_min, testimate);
        estimate_max = std::max(estimate_max, testimate);
        estimate_avg += testimate;
    }
    estimate_avg /= count;

    estimate = (3 * estimate_avg + estimate_max) / 4;

    return estimate;
}

void FirProcessor::UpdateFilterCoefficients(bool force) {
    const auto& output_port = m_output_port->GetPortInfo();

    const auto sample_size = sizeof(float);
    const auto new_channel_count = output_port.channel_count;
    const auto buffer_length = output_port.capacity_in_bytes / getSampleBytes(output_port.data_type);

    const uint32_t new_grain = std::min<uint32_t>(buffer_length, m_max_grain);
    if (m_real_grain != new_grain || m_channel_count != new_channel_count || force) {
        m_real_grain = new_grain;
        m_channel_count = new_channel_count;

        if (m_current_ir_filter->GetFilterLength() < FftParameters::config::fft_length) {
            // use as many samples of the input as possible
            m_fir_samples_per_segment = m_current_ir_filter->GetFilterLength();
            m_input_size_per_iteration = std::max(m_real_grain, 2 * FftParameters::config::fft_length - m_current_ir_filter->GetFilterLength());
        }
        else {
            // simply use half split
            m_input_size_per_iteration = FftParameters::config::fft_length;
            m_fir_samples_per_segment = FftParameters::config::fft_length;
        }

        m_segment_count = divup(m_current_ir_filter->GetFilterLength(), m_fir_samples_per_segment);
        m_max_overlap = std::min<uint32_t>(m_current_ir_filter->GetFilterLength(), 2 * FftParameters::config::fft_length);

        // allocate the device buffers
        const size_t inputsegmentstoragesize = static_cast<size_t>(m_segment_count) * FftParameters::config::fft_length * m_channel_count * sample_size * 2;
        if (m_fourier_input_segments_length < inputsegmentstoragesize) {
            m_fourier_input_segments = m_memory_manager.AllocateGpuMemory(inputsegmentstoragesize);
            m_fourier_input_segments_length = inputsegmentstoragesize;
        }

        const size_t overlapstoragesize = static_cast<size_t>(m_max_overlap) * m_channel_count * sample_size;
        if (m_overlap_length < overlapstoragesize) {
            m_overlap = m_memory_manager.AllocateGpuMemory(overlapstoragesize);
            m_overlap_length = overlapstoragesize;
        }

#ifdef SHARED_IRS
        // ensure IR buffers are allocated
        const uint32_t filterLength = m_current_ir_filter->GetFilterLength() * sample_size;
        m_current_ir_filter->getRawIR(0);
        m_real_filter_length = filterLength;

        const uint32_t firSegmentLengths = m_segment_count * FftParameters::config::fft_length * sample_size * 2;
        m_current_ir_filter->getSegments(0, firSegmentLengths);
        m_fourier_impulse_response_segments_length = firSegmentLengths;
#else
        for (size_t channel = m_current_ir_filter->GetChannelCount(); channel < MAX_CHANNELS; ++channel) {
            m_real_filter[channel].reset();
            m_fourier_impulse_response_segments[channel].reset();
        }

        const uint32_t filterLength = m_current_ir_filter->GetFilterLength() * sample_size;

        if (m_real_filter_length < filterLength) {
            for (size_t channel = 0; channel < m_current_ir_filter->GetChannelCount(); ++channel) {
                m_real_filter[channel] = m_memory_manager.AllocateGpuMemory(filterLength);
            }
        }
        m_real_filter_length = filterLength;

        for (size_t channel = 0; channel < m_current_ir_filter->GetChannelCount(); ++channel) {
            if (!m_real_filter[channel]) {
                m_real_filter[channel] = m_memory_manager.AllocateGpuMemory(filterLength);
            }
            m_memory_manager.MemCpyCpuToGpu(*m_real_filter[channel], 0, &m_current_ir_filter->GetValueAt(channel, 0), filterLength);
        }

        const uint32_t firSegmentLengths = m_segment_count * FftParameters::config::fft_length * sample_size * 2;
        if (m_fourier_impulse_response_segments_length < firSegmentLengths) {
            for (size_t channel = 0; channel < m_current_ir_filter->GetChannelCount(); ++channel) {
                m_fourier_impulse_response_segments[channel] = m_memory_manager.AllocateGpuMemory(firSegmentLengths);
            }
        }
        else {
            for (size_t channel = 0; channel < m_current_ir_filter->GetChannelCount(); ++channel)
                if (!m_fourier_impulse_response_segments[channel]) {
                    m_fourier_impulse_response_segments[channel] = m_memory_manager.AllocateGpuMemory(firSegmentLengths);
                }
        }
        m_fourier_impulse_response_segments_length = firSegmentLengths;
#endif

        m_recompute_filter = true;
    }
}

void FirProcessor::UpdateProcessorFilter(uint32_t choice) {
    m_current_ir_filter->LoadImpulseResponse(choice);
    UpdateFilterCoefficients(true);
}

FirProcessor::FirProcessor(::ProcessorSpecification& specification, ::Module& module) :
    m_module {module},
    m_proc_data {1u, sizeof(fir::ProcessorParameter), ProcessorEndCallback::eNoCallback, 1u, &m_gpu_task},
    m_port_factory {specification.port_factory},
    m_memory_manager {specification.memory_manager} {
    // Get the user-data for processor construction from the ProcessorSpecification
    auto spec = reinterpret_cast<const FirConfig::Specification*>(specification.user_data);
    // make sure the user-data is what we expect it to be, i.e., a FirConfig::Specification
    if (specification.data_size != sizeof(FirConfig::Specification) || spec->ThisType != FirConfig::Specification::FirConstructionType) {
        throw std::runtime_error("Error in FirProcessor::FirProcessor: invalid specification provided");
    }

    // specify what type of output port the processor has and create it
    PortInfo output_port_info {};
    output_port_info.type = PortType::eRegularPort;
    output_port_info.data_type = PortDataType::eSample32;
    output_port_info.capacity_in_bytes = m_real_grain;
    output_port_info.size_in_bytes = 0;
    output_port_info.channel_count = 1;
    output_port_info.grain = m_real_grain;
    m_output_port = m_port_factory.CreateDataPort(0u, output_port_info);

    m_current_ir_filter.reset(new MyIRFilter(m_memory_manager, spec->filter_length, spec->filter_index));
    UpdateProcessorFilter(spec->last_choice);

    // the processor only has one task/step, and its index is 0. See `DeclareProcessorStep` in `FirProcessor.cu`
    m_gpu_task.entry_idx = 0u;
    // number of threads required
    m_gpu_task.thread_count = FftParameters::config::fft_length_quarter;
    // how many blocks we launch into the process function
    m_gpu_task.block_count = output_port_info.channel_count;
    // required per-block shared memory for the task
    m_gpu_task.shared_mem_size = FftParameters::config::fft_sm_required * sizeof(float) * 2;
    // it does not take task parameters. see `using TaskParameter = void;` in `Properties.h`)
    m_gpu_task.task_param_size = 0u;
}
