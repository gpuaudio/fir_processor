/*
 * Copyright (c) 2022 Braingines SA - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef FIR_FIR_PROCESSOR_CUH
#define FIR_FIR_PROCESSOR_CUH

#include "Properties.h"

#include <gpu_primitives/FftCalculator.h>
#include <platform/Abstraction.h>

#if !defined(__METAL_DEVICE_COMPILE__)
#include <cstdio>
#else
#include <metal_integer>
using metal::min;
#endif

namespace FirProcessor {

template <typename T>
class FirProcessorDevice {
    int _segmentOffset[MAX_CHANNELS];      // points to the last input segment
    int _segmentZeroSamples[MAX_CHANNELS]; // samples as overlaps from last iteration

    static __program_scope constexpr int SymSize = FftParameters::config::fft_length;
    static __program_scope constexpr int BlockSize = FftParameters::config::fft_length_quarter;

public:
    // mandatory explicitly defined constructor
    __device_fct FirProcessorDevice() __device_addr {}

    // mandatory explicitly defined destructor
    __device_fct ~FirProcessorDevice() __device_addr {}

    // mandatory init function; can be used to initialize processor data members
    template <class TContext>
    __device_fct void init(TContext context, unsigned int maxBufferLength) __device_addr {
        for (int i = 0; i < MAX_CHANNELS; ++i) {
            _segmentOffset[i] = 0;
            _segmentZeroSamples[i] = 0;
        }
    }

    // Every task of the processor must match the following interface:
    // ```
    // template <class Context>
    // __device_fct void process(Context context, __device_addr fir::ProcessorParameter* processor_param, __device_addr fir::TaskParameter* task_param,
    //                           __device_addr float* __device_addr* input, __device_addr float* __device_addr* output) __device_addr
    // ```
    //
    // - `Context` provides all the call specific information, like `threadId`, `blockId`, `callId`, shared memory, synchronization ...
    //    - Depending on the scheduling environment and the task type, we specialize the `Context` as needed this is why it is a template parameter
    //
    // - `ProcessorParameter* processor_param` is a parameter that can be set per call of the processor
    //
    // - `TaskParameter* task_param` is a parameter specific to the task and is set for every task individually
    //    - All tasks get the same `ProcessorParameter` and individual `TaskParameters`
    //    - Simply set the size to 0 at the host interface and no space will be used for them
    //    - The host interface has to provide the sizes required for the parameters (see FirProcessor::m_proc_data and FirProcessor::m_gpu_task)
    //
    // - `float** input` points to the input. input[p][s] is sample s of port p.
    //    Layout: all samples of the first channel, all samples of the second channel, ...
    // - `float** output` points to allocated device memory for the output. output[p][s] is sample s of port p.
    //    Layout: all samples of the first channel, all samples of the second channel, ...
    //
    // ================================
    // Basic functionalities of `Context` are:
    //    - `call()` to get the call id : [0, FirProcessor::m_proc_data::num_calls - 1]
    //    - `blockId()` to get the blockId : [0, FirProcessor::m_gpu_task::block_count - 1]
    //    - `threadId()` to get the threadId : [0, FirProcessor::m_gpu_task::thread_count - 1]
    //    - `blockDim()` to get the blockSize : FirProcessor::m_gpu_task::thread_count
    //    - `smem()` to get the registered shared memory : FirProcessor::m_gpu_task::shared_mem_size bytes
    //    - `synchronize()` to synchronize all threads in the block
    //
    // Note:
    //  - exclusively use `context.synchronize()` for synchronization of a block. Platform specific sync operations might hang
    //  - use `context.blockId()`, `context.threadId()`, `context.blockDim()` instead of platform specific alternatives, which might be wrong
    //  - you can use multiple blocks,e.g., one or multiple per channel - Make sure in the host processor that enough blocks are requested (FirProcessor::m_gpu_task::block_count)
    //  - you can use multiple calls to split the processing of a longer buffer into smaller grain-sized portions. You can also do that with a loop inside the task.
    //    However, when you use multiple calls execution can parallelize better if there are multiple processors in the chain, as we can already execute the next processor
    //    when parts of its input, i.e., the current processors output, are available. It will guarantee that, within a processor, a grain-sized portion of the input
    //    will only be processed when the previous portion has been processed.

    template <class TContext>
    __device_fct void process(TContext context, __device_addr fir::ProcessorParameter* params, __device_addr fir::TaskParameter* task_param, __device_addr T* __device_addr* input, __device_addr T* __device_addr* output) __device_addr {
        if (params->filter_length_to_translate_init && context.call() == 0) [[unlikely]] {
            init(context, params);
        }

        int cursor;
        if (_segmentZeroSamples[context.blockId()] != 0) {
            int processSamples = min(params->input_length - (int)context.call() * params->grain, min(params->grain, params->input_samples_per_iteration - _segmentZeroSamples[context.blockId()]));
            int dataOffset = context.blockId() * params->input_length + context.call() * params->grain;
            processInternal(context, input[0] + dataOffset, output[0] + dataOffset,
                params->fourier_input_segments + params->segments_count * SymSize * context.blockId(),
                params->overlap + params->overlap_length * context.blockId(),
                params->fourier_impulse_response_segments[context.blockId()], processSamples, context.blockId(),
                params->segments_count, params->input_samples_per_iteration, params->overlap_length);
            cursor = processSamples;
        }
        else
            cursor = 0;

        for (; cursor < params->grain; cursor += params->input_samples_per_iteration) {
            int processSamples = min(min(params->input_length - (int)context.call() * params->grain, params->grain) - cursor, params->input_samples_per_iteration);
            int dataOffset = context.blockId() * params->input_length + cursor + context.call() * params->grain;
            processInternal(context, input[0] + dataOffset, output[0] + dataOffset,
                params->fourier_input_segments + params->segments_count * SymSize * context.blockId(),
                params->overlap + params->overlap_length * context.blockId(),
                params->fourier_impulse_response_segments[context.blockId()], processSamples, context.blockId(),
                params->segments_count, params->input_samples_per_iteration, params->overlap_length);
        }
    }

private:
    ////////////////////////////////////////////////////////

    class ComplexAccumulator {
        float2 _v[4] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
        __device_fct __forceinline_fct float2 mul(const __thread_addr float2& a, const __thread_addr float2& b) __thread_addr {
            return make_float2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
        }

    public:
        template <class TContext>
        __device_fct void multiplyAddFourierSym(__thread_addr TContext& context, const __threadgroup_addr float2* a, const __device_addr float2* b) __thread_addr {
#pragma unroll
            for (int i = 0; i < 4; ++i) {
                int idx = i * FftParameters::config::fft_length_quarter + context.threadId();
                float2 ai = a[idx];
                float2 bi = b[idx];
                float2 res = mul(ai, bi);

                if (i == 0)
                    if (context.threadId() == 0)
                        res = make_float2(ai.x * bi.x, ai.y * bi.y);
                _v[i].x += res.x;
                _v[i].y += res.y;
            }
        }
#if defined(__METAL_DEVICE_COMPILE__)
        template <class TContext>
        __device_fct void multiplyAddFourierSym(__thread_addr TContext& context, const __device_addr float2* a, const __device_addr float2* b) __thread_addr {
#pragma unroll
            for (int i = 0; i < 4; ++i) {
                int idx = i * FftParameters::config::fft_length_quarter + context.threadId();
                float2 ai = a[idx];
                float2 bi = b[idx];
                float2 res = mul(ai, bi);

                if (i == 0)
                    if (context.threadId() == 0)
                        res = make_float2(ai.x * bi.x, ai.y * bi.y);
                _v[i].x += res.x;
                _v[i].y += res.y;
            }
        }
#endif

        template <class TContext>
        __device_fct void expandToShared(__thread_addr TContext& context, __threadgroup_addr float2* s_input) __thread_addr {
#pragma unroll
            for (int i = 0; i < 4; ++i) {
                int idx = i * FftParameters::config::fft_length_quarter + context.threadId();
                s_input[idx] = _v[i];
            }
        }
    };

    __device_fct static float2 checkedLoad(const __device_addr T* input, int id, int length, int offset = 0) {
        if (id >= offset && id < offset + length - 1)
            return make_float2(input[id - offset], input[id - offset + 1]);
        else
            return make_float2(0, 0);
    }

    template <class TContext>
    __device_fct static __threadgroup_addr float2* loadInputToSharedChecked(__thread_addr TContext& context, const __device_addr T* input, int length, int offset = 0) {
        __threadgroup_addr float2* s_input = context.template smem_offset<float2>(0);
#pragma unroll
        for (int i = 0; i < 4; ++i) {
            int idx = i * FftParameters::config::fft_length_quarter + context.threadId();
            s_input[idx] = checkedLoad(input, idx * 2, length, offset);
        }
        return s_input;
    }

    ////////////////////////////////////////////////////////

    template <class TContext>
    __device_fct void processInternal(__thread_addr TContext& context, const __device_addr T* input, __device_addr T* output, __device_addr float2* fourierInputSegments,
        __device_addr T* overlap, const __device_addr float2* fourierImpulseResponseSegments, int inputSize, int channel,
        int segmentsCount, int inputSamplesPerIteration, int overlapLength) __device_addr {
        static_assert(FftParameters::config::fft_length >= 128, "Only Supporting for now");

        // fft of new segment to shared
        __threadgroup_addr float2* s_input = loadInputToSharedChecked(context, input, inputSize, _segmentZeroSamples[channel]);
        context.synchronize();

        dsp::FftCalculator<float>::template processR2C<FftParameters::config::fft_length * 2>(context, (__threadgroup_addr float*)s_input, (__threadgroup_addr float*)s_input);
        context.synchronize();

        ComplexAccumulator accumulator {};
        if (_segmentZeroSamples[channel] == 0) {
            for (int i = 1; i < segmentsCount; ++i)
                accumulator.multiplyAddFourierSym(context, fourierInputSegments + ((_segmentOffset[channel] + i) % segmentsCount) * SymSize, fourierImpulseResponseSegments + i * SymSize);
            context.synchronize();

            ComplexAccumulator tempAccumulator = accumulator;
            // add the new segment
            accumulator.multiplyAddFourierSym(context, s_input, fourierImpulseResponseSegments);
            context.synchronize();

            if (segmentsCount > 1 || inputSize != inputSamplesPerIteration) {
                // write the fourier transformed input segment to memory
#pragma unroll
                for (int i = 0; i < 4; ++i) {
                    int idx = i * FftParameters::config::fft_length_quarter + context.threadId();
                    fourierInputSegments[idx + _segmentOffset[channel] * SymSize] = s_input[idx];
                }
            }
            context.synchronize();

            if (segmentsCount > 1 && inputSize != inputSamplesPerIteration) {
                // the combined filters to this point need to be added to the overlap

                // convert from symmetric only part
                tempAccumulator.expandToShared(context, s_input);
                context.synchronize();

                // backward FFT
                dsp::FftCalculator<float>::template processC2R<FftParameters::config::fft_length * 2>(context, (__threadgroup_addr float*)s_input, (__threadgroup_addr float*)s_input);
                context.synchronize();

                // store the filter overlap for this segment
                for (int i = context.threadId() + inputSize; i < overlapLength; i += BlockSize) {
                    overlap[i] += ((__threadgroup_addr float*)s_input)[i];
                }
                context.synchronize();
            }
        }
        else {
            // add the fourier transformed input segment to memory
#pragma unroll
            for (int i = 0; i < 4; ++i) {
                int idx = i * FftParameters::config::fft_length_quarter + context.threadId();
                __device_addr float2& aStorage = fourierInputSegments[idx + _segmentOffset[channel] * SymSize];
                __threadgroup_addr float2& aLocal = s_input[idx];
                float2 a = aStorage;
                float2 aL = aLocal;
                aStorage = aLocal = make_float2(a.x + aL.x, a.y + aL.y);
            }
            context.synchronize();

            // add the new segment
            accumulator.multiplyAddFourierSym(context, s_input, fourierImpulseResponseSegments);
            context.synchronize();
        }

        // new first segment is second last segments
        if (context.threadId() == 0 && _segmentZeroSamples[channel] + inputSize == inputSamplesPerIteration)
            _segmentOffset[channel] = (_segmentOffset[channel] - 1 + segmentsCount) % segmentsCount;

        // convert from symmetric only part
        accumulator.expandToShared(context, s_input);
        context.synchronize();

        // backward FFT
        dsp::FftCalculator<float>::template processC2R<FftParameters::config::fft_length * 2>(context, (__threadgroup_addr float*)s_input, (__threadgroup_addr float*)s_input);
        context.synchronize();

        // write result out
        for (int i = context.threadId(); i < inputSize; i += BlockSize) {
            int relative = _segmentZeroSamples[channel] + i;
            T overlapValue = 0;
            if (relative < overlapLength)
                overlapValue = overlap[relative];
            output[i] = ((__threadgroup_addr float*)s_input)[relative] + overlapValue;
        }
        context.synchronize();

        // store the new overlap (potentially combine with leftover overlap)
        if (_segmentZeroSamples[channel] + inputSize == inputSamplesPerIteration) {
            for (int i = 0; i < overlapLength; i += BlockSize) {
                T accValue = 0;
                if (i + inputSamplesPerIteration < overlapLength) {
                    int prevOverlapId = i + inputSamplesPerIteration + context.threadId();
                    if (prevOverlapId < overlapLength)
                        accValue = overlap[prevOverlapId];
                    context.synchronize();
                }
                int resultId = i + inputSamplesPerIteration + context.threadId();
                int outId = i + context.threadId();
                if (resultId < 2 * FftParameters::config::fft_length)
                    accValue += ((__threadgroup_addr float*)s_input)[resultId];

                if (outId < overlapLength)
                    overlap[outId] = accValue;
            }
        }
        context.synchronize();

        if (context.threadId() == 0)
            _segmentZeroSamples[channel] = (_segmentZeroSamples[channel] + inputSize) % inputSamplesPerIteration;
        context.synchronize();
    }

    template <class TContext>
    __device_fct void init(__thread_addr TContext& context, __device_addr fir::ProcessorParameter* params) __device_addr {
        // initFilter
        __device_addr float2* pResponseSegments = const_cast<__device_addr float2*>(params->fourier_impulse_response_segments[context.blockId()]);

        if (context.threadId() == 0) {
            _segmentZeroSamples[context.blockId()] = (params->init_buffer_offset) % params->input_samples_per_iteration;
        }

        for (int offset = 0; offset < params->filter_length_to_translate_init; offset += params->fir_samples_per_iteration) {
            context.synchronize();

            __threadgroup_addr float2* s_input = loadInputToSharedChecked(context, params->real_filter[context.blockId()] + offset, min(params->fir_samples_per_iteration, params->filter_length_to_translate_init - offset));
            context.synchronize();

            dsp::FftCalculator<float>::template processR2C<FftParameters::config::fft_length * 2>(context, (__threadgroup_addr float*)s_input, (__threadgroup_addr float*)s_input);
            context.synchronize();

#pragma unroll
            for (int i = 0; i < 4; ++i) {
                int idx = i * FftParameters::config::fft_length_quarter + context.threadId();
                pResponseSegments[idx] = s_input[idx];
            }

            pResponseSegments += SymSize;
        }

        // initSignalSegments
        for (int i = context.threadId(); i < params->segments_count * SymSize; i += context.blockDim())
            params->fourier_input_segments[params->segments_count * SymSize * context.blockId() + i] = make_float2(0, 0);
        for (int i = context.threadId(); i < params->overlap_length; i += context.blockDim())
            params->overlap[params->overlap_length * context.blockId() + i] = 0;
    }
};
} // namespace FirProcessor

#endif // FIR_FIR_PROCESSOR_CUH
