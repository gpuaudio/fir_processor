/*
 * Copyright (c) 2022 Braingines SA - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef FIR_FIR_PROCESSOR_H
#define FIR_FIR_PROCESSOR_H

#include "device/Properties.h"
#include "IRFilter.h"
#include "convolution_filter/StaticIRShare.h"

#include <fir_processor/FirSpecification.h>

#include <processor_api/GpuTaskData.h>
#include <processor_api/InputPort.h>
#include <processor_api/LaunchData.h>
#include <processor_api/MemoryManager.h>
#include <processor_api/ModuleBase.h>
#include <processor_api/OutputPort.h>
#include <processor_api/Processor.h>
#include <processor_api/ProcessorBlueprint.h>
#include <processor_api/ProcessorProfiler.h>
#include <processor_api/PortFactory.h>

#include <cstdint>
#include <fstream>
#include <memory>

class FirProcessor : public GPUA::processor::v2::Processor, public GPUA::processor::v2::InputPort, private GPUA::processor::v2::ProcessorProfiler {
public:
    explicit FirProcessor(GPUA::processor::v2::ProcessorSpecification& specification, GPUA::processor::v2::Module& module);

    ~FirProcessor() = default;

    // Copy ctor and copy assignment are deleted along with move assignment operator deletion
    FirProcessor& operator=(FirProcessor&) = delete;

    // GPUA::processor::v2::Processor methods
    GPUA::processor::v2::Module& GetModule() const noexcept override;

    GPUA::processor::v2::ErrorCode SetData(void* data, uint32_t data_size) noexcept override;
    GPUA::processor::v2::ErrorCode GetData(void* data, uint32_t& data_size) const noexcept override;

    uint32_t GetInputPortCount() const noexcept override;
    GPUA::processor::v2::ErrorCode GetInputPort(uint32_t index, GPUA::processor::v2::InputPort*& port) noexcept override;

    GPUA::processor::v2::ErrorCode OnBlueprintRebuild(const GPUA::processor::v2::ProcessorBlueprint*& blueprint) noexcept override;
    GPUA::processor::v2::ErrorCode PrepareForProcess(const GPUA::processor::v2::LaunchData& data, uint32_t expected_chunks) noexcept override;
    GPUA::processor::v2::ErrorCode PrepareChunk(void* proc_data, void** task_data, uint32_t chunk_id) noexcept override;
    void OnProcessingEnd(bool after_fat_transfer) noexcept override;

    // GPUA::processor::v2::InputPort
    GPUA::processor::v2::PortId GetPortId() noexcept override;
    GPUA::processor::v2::ErrorCode Connect(const GPUA::processor::v2::OutputPort& data_port) noexcept override;
    GPUA::processor::v2::ErrorCode Disconnect() noexcept override;
    GPUA::processor::v2::ErrorCode InputPortUpdated(GPUA::processor::v2::PortChangedFlags flags, const GPUA::processor::v2::OutputPort& data_port) noexcept override;
    uint32_t GetInputGrain() const noexcept override;
    GPUA::processor::v2::ErrorCode GetPortDescription(const GPUA::processor::v2::PortDescription*& description) const noexcept override;

    // GPUA::processor::v2::ProcessorProfiler
    GPUA::processor::v2::ProcessorProfiler* GetProcessorProfiler() noexcept override;

private:
    uint32_t RunProfiling(const GPUA::processor::v2::ProfileSpecification& spec, GPUA::processor::v2::LatencyProfiler& profiler) noexcept override;

    void UpdateFilterCoefficients(bool force = false);
    void UpdateProcessorFilter(uint32_t choice);

    GPUA::processor::v2::Module& m_module;
    GPUA::processor::v2::PortFactory& m_port_factory;
    GPUA::processor::v2::MemoryManager& m_memory_manager;

    GPUA::processor::v2::GpuTaskData m_gpu_task {};
    GPUA::processor::v2::ProcessorBlueprint m_proc_data;

    GPUA::processor::v2::OutputPortPointer m_output_port {0, 0};

    bool m_changed {true};
    bool m_initialized {false};
    bool m_recompute_filter {true};

    uint32_t m_channel_count {1};
    uint32_t m_max_grain {FftParameters::config::fft_length};
    uint32_t m_real_grain {0}; // real -> actual

    // inputsize per iteration = FFTwidth, filter samples per segment = FFTwidth
    uint32_t m_input_size_per_iteration {FftParameters::config::fft_length};
    uint32_t m_fir_samples_per_segment {FftParameters::config::fft_length};
    uint32_t m_segment_count {1};

    // if we know the grain, we can determine the min input samples per iteration (max difference of multiples of InputSize and FFT)
    uint32_t m_max_overlap {FftParameters::config::fft_length};

    GPUA::processor::v2::MemoryManager::GpuMemoryPointer m_fourier_input_segments {0, 0};
    GPUA::processor::v2::MemoryManager::GpuMemoryPointer m_overlap {0, 0};
    uint32_t m_fourier_input_segments_length {0};
    uint32_t m_overlap_length {0};

    uint32_t m_fourier_impulse_response_segments_length {0};
    uint32_t m_old_choice {};

#ifdef SHARED_IRS
    using MyIRFilter = StaticIRShare;
    std::unique_ptr<MyIRFilter> m_current_ir_filter {nullptr};
#else
    using MyIRFilter = IRFilter;
    std::unique_ptr<IRFilter> m_current_ir_filter {nullptr};
    GPUA::processor::v2::MemoryManager::GpuMemoryPointer m_fourier_impulse_response_segments[MAX_CHANNELS];
    GPUA::processor::v2::MemoryManager::GpuMemoryPointer m_real_filter[MAX_CHANNELS]; // real -> audio signal
#endif

    uint32_t m_real_filter_length {0};
};

#endif // FIR_FIR_PROCESSOR_H
