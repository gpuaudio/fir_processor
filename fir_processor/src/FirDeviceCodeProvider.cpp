/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#include "FirDeviceCodeProvider.h"

#include "cmrc/cmrc.hpp"

#include <codecvt>
#include <iostream>
#include <locale>
#include <string>

CMRC_DECLARE(BG::fir_processor);

namespace {

#if WIN32
static const std::string g_fir_code_prefix = "";
#else
static const std::string g_fir_code_prefix = "lib";
#endif

static const std::string g_fir_code_filename = g_fir_code_prefix + "fir_processor.";
#if defined(GPU_AUDIO_NV)
static const std::string g_fir_code_file_ext = ".cubin";
#elif defined(GPU_AUDIO_AMD)
static const std::string g_fir_code_file_ext = ".o";
#elif defined(GPU_AUDIO_MAC)
static const std::string g_fir_code_file_ext = ".metallib";
#endif

} // namespace

FirDeviceCodeProvider::FirDeviceCodeProvider(const GPUA::processor::v2::DeviceCodeSpecification& specification) :
    m_platform {specification.platform} {
}

GPUA::processor::v2::ErrorCode FirDeviceCodeProvider::GetDeviceCode(GPUA::processor::v2::InputStream*& input_stream) noexcept {
    // convert gpu platform arch from wstring to string
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string platform_str = converter.to_bytes(m_platform);

    // assemble device code filename
    std::string filename {g_fir_code_filename + platform_str + g_fir_code_file_ext};

    try {
        // read device code to binary stream
        auto fs = cmrc::BG::fir_processor::get_filesystem();
        auto file = fs.open(filename);
        const auto size = std::distance(file.begin(), file.end());
        m_stream = std::make_unique<StreamAdapter>(file.begin(), size);
        input_stream = m_stream.get();
        return GPUA::processor::v2::ErrorCode::eSuccess;
    }
    catch (const std::exception& exc) {
        std::cout << exc.what() << std::endl;
    }

    input_stream = nullptr;
    return GPUA::processor::v2::ErrorCode::eFail;
}
