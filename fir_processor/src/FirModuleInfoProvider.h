/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef FIR_FIR_MODULE_INFO_PROVIDER_H
#define FIR_FIR_MODULE_INFO_PROVIDER_H

#include <processor_api/ModuleBase.h>
#include <processor_api/ModuleInfoProvider.h>

#include <vector>

class FirModuleInfoProvider : public GPUA::processor::v2::ModuleInfoProvider {
public:
    FirModuleInfoProvider();
    ~FirModuleInfoProvider() override = default;

    // Copy ctor and copy assignment op are deleted along with move assignment operator
    FirModuleInfoProvider& operator=(FirModuleInfoProvider&&) noexcept = delete;

    ////////////////////////////////
    // GPUA::processor::v2::ModuleInfoProvider methods
    uint32_t GetSupportPlatformCount() const noexcept override;
    GPUA::processor::v2::ErrorCode GetSupportPlatformInfo(uint32_t index, const GPUA::processor::v2::PlatformInfo*& platform_info) const noexcept override;
    GPUA::processor::v2::ErrorCode GetModuleInfo(const GPUA::processor::v2::ModuleInfo*& module_info) const noexcept override;
    GPUA::processor::v2::ErrorCode GetProcessorExecutionInfo(const GPUA::processor::v2::ProcessorEntryInfo*& entry_info) const noexcept override;
    // GPUA::processor::v2::ModuleInfoProvider methods
    ////////////////////////////////

private:
    static const std::vector<GPUA::processor::v2::PlatformInfo>& GetCodeSpecs();

    static constexpr const wchar_t* EffectName {L"" MODULE_EFFECT_NAME};
    static constexpr const wchar_t* ModuleId {L"" MODULE_ID};
    static constexpr const GPUA::processor::v2::Version Version {MODULE_MAJOR_VERSION, MODULE_MINOR_VERSION, MODULE_PATCH_LEVEL};
    static constexpr const GPUA::processor::v2::ModuleInfo ModuleInfo {Version, EffectName, ModuleId};
};

#endif // FIR_FIR_MODULE_INFO_PROVIDER_H
