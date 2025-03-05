/*
 * Copyright (c) 2022 Braingines SA - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef FIR_PROPERTIES_H
#define FIR_PROPERTIES_H

#if defined(GPU_AUDIO_AMD)
#include <hip/hip_runtime.h>
#elif defined(GPU_AUDIO_NV)
#include <cuda_runtime.h>
#endif

#include <platform/Abstraction.h>

#define SHARED_IRS

// The entries in GPUFUNCTIONS_SCRAMBLED are used to replace the processor device function names
// during compilation to avoid name conflicts between processors.
// clang-format off
#define GPUFUNCTIONS_SCRAMBLED \
jtrsacDFwbKjuk2ULx28, \
JonZRaCstui141WITrbu, \
dayzDahhZujai8UR2a9T, \
qK2F6BIOWyfXGnjiqATo, \
hJkKfAEdl7G7NVbeO5gJ, \
YS3jRTDPrNDhZohM0vBh, \
rv18b4FUn1JcELdIBaPn, \
nZhpER8T7QmHOZi7LGau, \
YMvzEVWpaivpX5byw4ys, \
PP3YSq7ouIVJRW5tfYQ8
// clang-format on

// DO NOT REMOVE! Contains macros for device function name substitution.
#include <scheduler/common_macros.h>

#include "SM_FFT_parameters.cuh" // TODO: we should use <dsp_library/FftCalculatorParams.h>

// NOTE that you need to decrease the max registers for higher FFT sizes
// always use 128 max register
// for 4096 you need 64!!
#if defined(GPU_AUDIO_MAC)
__program_scope constexpr int FFT_WIDTH = 1024;
#else
__program_scope constexpr int FFT_WIDTH = 2048;
#endif
__program_scope constexpr int MAX_CHANNELS = 2;

using FftParameters = FFTParameters<FFT_WIDTH, false, true>;

// parameter struct passed to each task (members are set in FirProcessor::PrepareChunk)
namespace fir {
struct ProcessorParameter {
    __device_addr float2* fourier_input_segments;
    __device_addr float* overlap;
    const __device_addr float2* fourier_impulse_response_segments[MAX_CHANNELS];
    const __device_addr float* real_filter[MAX_CHANNELS]; // real -> audio

    int segments_count;
    int input_samples_per_iteration;
    int fir_samples_per_iteration;
    int overlap_length;

    int input_length;
    int filter_length_to_translate_init;
    int grain;
    int init_buffer_offset;
};

// per task parameter struct. could be different for each task if the processor
// had more than one. unused in fir_processor.
using TaskParameter = void;
} // namespace fir

#endif // FIR_PROPERTIES_H
