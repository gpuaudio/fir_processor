#ifndef SM_FFT_PARAMETERS_H
#define SM_FFT_PARAMETERS_H

#include <platform/Abstraction.h>

struct FFT_Params {
    static __program_scope constexpr int fft_exp {-1};
    static __program_scope constexpr int fft_length {-1};
    static __program_scope constexpr int warp {32};
};

struct FFT_32_forward {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {5};
    static __program_scope constexpr int fft_sm_required {128};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_32_forward_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {5};
    static __program_scope constexpr int fft_sm_required {128};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_32_inverse {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {5};
    static __program_scope constexpr int fft_sm_required {128};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_32_inverse_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {5};
    static __program_scope constexpr int fft_sm_required {128};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_64_forward {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {6};
    static __program_scope constexpr int fft_sm_required {132};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_64_forward_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {6};
    static __program_scope constexpr int fft_sm_required {132};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_64_inverse {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {6};
    static __program_scope constexpr int fft_sm_required {132};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_64_inverse_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {6};
    static __program_scope constexpr int fft_sm_required {132};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_128_forward {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {7};
    static __program_scope constexpr int fft_sm_required {132};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_128_forward_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {7};
    static __program_scope constexpr int fft_sm_required {132};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_128_inverse {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {7};
    static __program_scope constexpr int fft_sm_required {132};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_128_inverse_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {7};
    static __program_scope constexpr int fft_sm_required {132};
    static __program_scope constexpr int fft_length {128};
    static __program_scope constexpr int fft_length_quarter {32};
    static __program_scope constexpr int fft_length_half {64};
    static __program_scope constexpr int fft_length_three_quarters {96};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_256_forward {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {8};
    static __program_scope constexpr int fft_sm_required {264};
    static __program_scope constexpr int fft_length {256};
    static __program_scope constexpr int fft_length_quarter {64};
    static __program_scope constexpr int fft_length_half {128};
    static __program_scope constexpr int fft_length_three_quarters {192};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_256_forward_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {8};
    static __program_scope constexpr int fft_sm_required {264};
    static __program_scope constexpr int fft_length {256};
    static __program_scope constexpr int fft_length_quarter {64};
    static __program_scope constexpr int fft_length_half {128};
    static __program_scope constexpr int fft_length_three_quarters {192};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_256_inverse {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {8};
    static __program_scope constexpr int fft_sm_required {264};
    static __program_scope constexpr int fft_length {256};
    static __program_scope constexpr int fft_length_quarter {64};
    static __program_scope constexpr int fft_length_half {128};
    static __program_scope constexpr int fft_length_three_quarters {192};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_256_inverse_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {8};
    static __program_scope constexpr int fft_sm_required {264};
    static __program_scope constexpr int fft_length {256};
    static __program_scope constexpr int fft_length_quarter {64};
    static __program_scope constexpr int fft_length_half {128};
    static __program_scope constexpr int fft_length_three_quarters {192};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_512_forward {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {9};
    static __program_scope constexpr int fft_sm_required {528};
    static __program_scope constexpr int fft_length {512};
    static __program_scope constexpr int fft_length_quarter {128};
    static __program_scope constexpr int fft_length_half {256};
    static __program_scope constexpr int fft_length_three_quarters {384};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_512_forward_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {9};
    static __program_scope constexpr int fft_sm_required {528};
    static __program_scope constexpr int fft_length {512};
    static __program_scope constexpr int fft_length_quarter {128};
    static __program_scope constexpr int fft_length_half {256};
    static __program_scope constexpr int fft_length_three_quarters {384};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_512_inverse {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {9};
    static __program_scope constexpr int fft_sm_required {528};
    static __program_scope constexpr int fft_length {512};
    static __program_scope constexpr int fft_length_quarter {128};
    static __program_scope constexpr int fft_length_half {256};
    static __program_scope constexpr int fft_length_three_quarters {384};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_512_inverse_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {9};
    static __program_scope constexpr int fft_sm_required {528};
    static __program_scope constexpr int fft_length {512};
    static __program_scope constexpr int fft_length_quarter {128};
    static __program_scope constexpr int fft_length_half {256};
    static __program_scope constexpr int fft_length_three_quarters {384};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_1024_forward {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {10};
    static __program_scope constexpr int fft_sm_required {1056};
    static __program_scope constexpr int fft_length {1024};
    static __program_scope constexpr int fft_length_quarter {256};
    static __program_scope constexpr int fft_length_half {512};
    static __program_scope constexpr int fft_length_three_quarters {768};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_1024_forward_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {10};
    static __program_scope constexpr int fft_sm_required {1056};
    static __program_scope constexpr int fft_length {1024};
    static __program_scope constexpr int fft_length_quarter {256};
    static __program_scope constexpr int fft_length_half {512};
    static __program_scope constexpr int fft_length_three_quarters {768};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_1024_inverse {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {10};
    static __program_scope constexpr int fft_sm_required {1056};
    static __program_scope constexpr int fft_length {1024};
    static __program_scope constexpr int fft_length_quarter {256};
    static __program_scope constexpr int fft_length_half {512};
    static __program_scope constexpr int fft_length_three_quarters {768};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_1024_inverse_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {10};
    static __program_scope constexpr int fft_sm_required {1056};
    static __program_scope constexpr int fft_length {1024};
    static __program_scope constexpr int fft_length_quarter {256};
    static __program_scope constexpr int fft_length_half {512};
    static __program_scope constexpr int fft_length_three_quarters {768};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_2048_forward {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {11};
    static __program_scope constexpr int fft_sm_required {2112};
    static __program_scope constexpr int fft_length {2048};
    static __program_scope constexpr int fft_length_quarter {512};
    static __program_scope constexpr int fft_length_half {1024};
    static __program_scope constexpr int fft_length_three_quarters {1536};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_2048_forward_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {11};
    static __program_scope constexpr int fft_sm_required {2112};
    static __program_scope constexpr int fft_length {2048};
    static __program_scope constexpr int fft_length_quarter {512};
    static __program_scope constexpr int fft_length_half {1024};
    static __program_scope constexpr int fft_length_three_quarters {1536};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_2048_inverse {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {11};
    static __program_scope constexpr int fft_sm_required {2112};
    static __program_scope constexpr int fft_length {2048};
    static __program_scope constexpr int fft_length_quarter {512};
    static __program_scope constexpr int fft_length_half {1024};
    static __program_scope constexpr int fft_length_three_quarters {1536};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_2048_inverse_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {11};
    static __program_scope constexpr int fft_sm_required {2112};
    static __program_scope constexpr int fft_length {2048};
    static __program_scope constexpr int fft_length_quarter {512};
    static __program_scope constexpr int fft_length_half {1024};
    static __program_scope constexpr int fft_length_three_quarters {1536};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_4096_forward {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {12};
    static __program_scope constexpr int fft_sm_required {4224};
    static __program_scope constexpr int fft_length {4096};
    static __program_scope constexpr int fft_length_quarter {1024};
    static __program_scope constexpr int fft_length_half {2048};
    static __program_scope constexpr int fft_length_three_quarters {3072};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_4096_forward_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {12};
    static __program_scope constexpr int fft_sm_required {4224};
    static __program_scope constexpr int fft_length {4096};
    static __program_scope constexpr int fft_length_quarter {1024};
    static __program_scope constexpr int fft_length_half {2048};
    static __program_scope constexpr int fft_length_three_quarters {3072};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

struct FFT_4096_inverse {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {12};
    static __program_scope constexpr int fft_sm_required {4224};
    static __program_scope constexpr int fft_length {4096};
    static __program_scope constexpr int fft_length_quarter {1024};
    static __program_scope constexpr int fft_length_half {2048};
    static __program_scope constexpr int fft_length_three_quarters {3072};
    static __program_scope constexpr int fft_direction {1};
    static __program_scope constexpr int fft_reorder {1};
};

struct FFT_4096_inverse_noreorder {
    static __program_scope constexpr FFT_Params fft_common_params {};
    static __program_scope constexpr int fft_exp {12};
    static __program_scope constexpr int fft_sm_required {4224};
    static __program_scope constexpr int fft_length {4096};
    static __program_scope constexpr int fft_length_quarter {1024};
    static __program_scope constexpr int fft_length_half {2048};
    static __program_scope constexpr int fft_length_three_quarters {3072};
    static __program_scope constexpr int fft_direction {0};
    static __program_scope constexpr int fft_reorder {0};
};

template <int Length, bool Inverse = false, bool Reorder = true>
struct FFTParameters;

template <>
struct FFTParameters<64, false, true> {
    using config = FFT_64_forward;
};
template <>
struct FFTParameters<64, true, true> {
    using config = FFT_64_inverse;
};

template <>
struct FFTParameters<128, false, true> {
    using config = FFT_128_forward;
};
template <>
struct FFTParameters<128, true, true> {
    using config = FFT_128_inverse;
};

template <>
struct FFTParameters<256, false, true> {
    using config = FFT_256_forward;
};
template <>
struct FFTParameters<256, true, true> {
    using config = FFT_256_inverse;
};

template <>
struct FFTParameters<512, false, true> {
    using config = FFT_512_forward;
};
template <>
struct FFTParameters<512, true, true> {
    using config = FFT_512_inverse;
};

template <>
struct FFTParameters<1024, false, true> {
    using config = FFT_1024_forward;
};
template <>
struct FFTParameters<1024, true, true> {
    using config = FFT_1024_inverse;
};

template <>
struct FFTParameters<2048, false, true> {
    using config = FFT_2048_forward;
};
template <>
struct FFTParameters<2048, true, true> {
    using config = FFT_2048_inverse;
};

template <>
struct FFTParameters<4096, false, true> {
    using config = FFT_4096_forward;
};
template <>
struct FFTParameters<4096, true, true> {
    using config = FFT_4096_inverse;
};

#endif /* SM_FFT_PARAMETERS_H */
