#include "StaticIRShare.h"

std::unordered_map<StaticIRShare::IRInfo, StaticIRShare::Data, StaticIRShare::IRInfo::Hasher> StaticIRShare::m_shared_irs;
std::mutex StaticIRShare::m_shared_ir_mutex;

namespace {
template <typename T>
T divup(T a, T b) {
    return (a + b - 1) / b;
}
template <typename T>
T align(T a, T b) {
    return divup(a, b) * b;
}
} // namespace

StaticIRShare::StaticIRShare(GPUA::processor::v2::MemoryManager& memory_manager, uint32_t filter_length, uint32_t filter_index) :
    m_ir_store(ImpulseResponseStore::GetInstance(filter_length, filter_index)),
    m_memory_manager(memory_manager),
    m_filter_length {filter_length},
    m_single_location {filter_index} {
}

StaticIRShare::~StaticIRShare() {
    unload();
}

unsigned int StaticIRShare::GetFilterLength() {
    return m_filter_length;
}

unsigned int StaticIRShare::GetChannelCount() {
    return m_channel_count;
}

void StaticIRShare::LoadImpulseResponse(int index) {
    if (m_filter_load_index == index) {
        return;
    }
    unload();

    if (index < 0) {
        return GenerateIR(-index, -index / 2);
    }

    m_filter_load_index = index;
    m_channel_count = m_ir_store.GetGainCompensatedIr(index).getNumChannels();

    // TODO: There's no guarantee that the size of each channel is the same, unfortunately. Fix this accordingly.
    m_filter_length = static_cast<uint32_t>(m_ir_store.GetGainCompensatedIr(index).samples[0].size());
}

void StaticIRShare::GenerateIR(uint32_t filter_length, uint32_t filter_index) {
    if (m_filter_load_index == 0xFFFFFFFF && filter_length == m_filter_length && filter_index == m_single_location) {
        return;
    }
    unload();
    m_filter_length = filter_length;
    m_single_location = filter_index;
}

void StaticIRShare::unload() {
    if (m_is_allocated) {
        std::unique_lock<std::mutex> m_lock(m_shared_ir_mutex);
        auto found = m_shared_irs.find({m_filter_load_index, m_filter_length, m_single_location});
        if (found != end(m_shared_irs)) {
            if (--found->second.m_refcounting == 0) {
                m_shared_irs.erase(found);
            }
        }
    }
    m_filter_load_index = 0xFFFFFFFF;
    m_filter_length = 0xFFFFFFFF;
    m_single_location = 0xFFFFFFFF;
    m_channel_count = 1;
    m_is_allocated = false;
    m_raw = 0;
    m_segments = 0;
}

GPUA::processor::v2::GpuPointer StaticIRShare::getRawIR(unsigned int channel) {
    if (!m_raw) {
        std::unique_lock<std::mutex> m_lock(m_shared_ir_mutex);
        auto found = m_shared_irs.find({m_filter_load_index, m_filter_length, m_single_location});
        m_raw_step = align<size_t>(m_filter_length * sizeof(float), 128U);

        if (found == end(m_shared_irs)) {
            found = m_shared_irs.insert(std::make_pair(IRInfo {m_filter_load_index, m_filter_length, m_single_location}, Data {})).first;
        }
        if (!found->second.m_gpu_raw) {
            found->second.m_gpu_raw = m_memory_manager.AllocateGpuMemory(m_raw_step * m_channel_count);

            if (m_filter_load_index == 0xFFFFFFFF) {
                std::vector<float> temp(m_filter_length, 0.0f);
                temp[m_single_location] = 1.0f;
                m_memory_manager.MemCpyCpuToGpu(*found->second.m_gpu_raw, 0, temp.data(), m_filter_length * sizeof(float));
            }
            else {
                for (unsigned channel = 0; channel < m_channel_count; ++channel) {
                    m_memory_manager.MemCpyCpuToGpu(*found->second.m_gpu_raw, channel * m_raw_step, m_ir_store.GetGainCompensatedIr(m_filter_load_index).samples[channel].data(), m_filter_length * sizeof(float));
                }
            }
        }
        m_raw = found->second.m_gpu_raw->GetGpuPointer();
        if (!m_is_allocated) {
            ++found->second.m_refcounting;
            m_is_allocated = true;
        }
    }

    if (channel >= m_channel_count) {
        channel = 0;
    }

    return m_raw + m_raw_step * channel;
}

GPUA::processor::v2::GpuPointer StaticIRShare::getSegments(unsigned int channel, unsigned int segmentlength) {
    if (!m_segments) {
        std::unique_lock<std::mutex> m_lock(m_shared_ir_mutex);
        auto found = m_shared_irs.find({m_filter_load_index, m_filter_length, m_single_location});
        m_segement_step = align<size_t>(segmentlength, 128U);

        if (found == end(m_shared_irs)) {
            found = m_shared_irs.insert(std::make_pair(IRInfo {m_filter_load_index, m_filter_length, m_single_location}, Data {})).first;
        }

        if (!found->second.m_gpu_segments) {
            found->second.m_gpu_segments = m_memory_manager.AllocateGpuMemory(m_segement_step * m_channel_count);
        }

        m_segments = found->second.m_gpu_segments->GetGpuPointer();
        if (!m_is_allocated) {
            ++found->second.m_refcounting;
            m_is_allocated = true;
        }
    }

    if (channel >= m_channel_count) {
        channel = 0;
    }

    return m_segments + m_segement_step * channel;
}
