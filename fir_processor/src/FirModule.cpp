/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#include "FirModule.h"
#include "FirProcessor.h"

FirModule::FirModule(const GPUA::processor::v2::ModuleSpecification& specification) :
    GPUA::processor::v2::ModuleBase(specification) {}

GPUA::processor::v2::ErrorCode FirModule::CreateProcessor(GPUA::processor::v2::ProcessorSpecification& specification, GPUA::processor::v2::Processor*& processor) noexcept {
    try {
        processor = new FirProcessor(specification, *this);
        return GPUA::processor::v2::ErrorCode::eSuccess;
    }
    catch (...) {
    }
    processor = nullptr;
    return GPUA::processor::v2::ErrorCode::eFail;
}

GPUA::processor::v2::ErrorCode FirModule::DeleteProcessor(GPUA::processor::v2::Processor* processor) noexcept {
    if (processor) {
        delete processor;
        return GPUA::processor::v2::ErrorCode::eSuccess;
    }
    return GPUA::processor::v2::ErrorCode::eFail;
}
