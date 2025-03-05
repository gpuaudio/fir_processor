/*
 * Copyright (c) 2022 Braingines SA - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef EARLYACCESSPRODUCT_STATIC_IR_SHARE_H
#define EARLYACCESSPRODUCT_STATIC_IR_SHARE_H

#include "../ImpulseResponseStore.h"

#include <processor_api/MemoryManager.h>

#include <cstdint>
#include <unordered_map>
#include <atomic>
#include <mutex>

class StaticIRShare {
public:
    StaticIRShare(GPUA::processor::v2::MemoryManager& memory_manager, uint32_t filter_length, uint32_t filter_index);
    ~StaticIRShare();
    StaticIRShare(const StaticIRShare&) = delete;
    StaticIRShare& operator=(const StaticIRShare&) = delete;

    unsigned int GetFilterLength();
    unsigned int GetChannelCount();

    void LoadImpulseResponse(int index);
    void GenerateIR(uint32_t filter_length, uint32_t filter_index);

    GPUA::processor::v2::GpuPointer getRawIR(unsigned int channel);
    GPUA::processor::v2::GpuPointer getSegments(unsigned int channel, unsigned int segmentlength);

private:
    ImpulseResponseStore& m_ir_store;
    GPUA::processor::v2::MemoryManager& m_memory_manager;

    struct IRInfo {
        uint32_t filterLoadIndex {0xFFFFFFFFu};
        uint32_t filterLength {0xFFFFFFFFu};
        uint32_t singleLocation {0xFFFFFFFFu};

        bool operator==(const IRInfo& other) const {
            return filterLoadIndex == other.filterLoadIndex && filterLength == other.filterLength && singleLocation == other.singleLocation;
        }

        struct Hasher {
            std::size_t operator()(const StaticIRShare::IRInfo& k) const {
                using std::hash;

                return ((hash<uint32_t>()(k.filterLoadIndex) ^ (hash<uint32_t>()(k.filterLength) << 1)) >> 1) ^ (hash<uint32_t>()(k.singleLocation) << 1);
            }
        };
    };

    struct Data {
        GPUA::processor::v2::GpuMemoryPointer m_gpu_raw {0, 0};
        GPUA::processor::v2::GpuMemoryPointer m_gpu_segments {0, 0};
        uint32_t m_refcounting {0u};
        Data() {}
    };

    static std::unordered_map<IRInfo, Data, IRInfo::Hasher> m_shared_irs;
    static std::mutex m_shared_ir_mutex;

    bool m_is_allocated {false};

    uint32_t m_filter_load_index {0xFFFFFFFFu};
    uint32_t m_filter_length {0xFFFFFFFFu};
    uint32_t m_single_location {0xFFFFFFFFu};
    uint32_t m_channel_count {1u};
    GPUA::processor::v2::GpuPointer m_raw {0};
    GPUA::processor::v2::GpuPointer m_segments {0};
    uint32_t m_raw_step;
    uint32_t m_segement_step;

    void unload();
};

#endif // EARLYACCESSPRODUCT_STATIC_IR_SHARE_H
