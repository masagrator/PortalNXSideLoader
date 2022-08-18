/**
 * @file oe.h
 * @brief Extenstions to OS functions.
 */

#pragma once

#include "settings.h"
#include "types.h"

namespace nn {
namespace oe {
    typedef s32 FocusHandlingMode;
    
    enum CpuBoostMode
    {
        Disabled,
        BoostCPU,
        ThrottleGPU,
    };

    enum PerformanceMode
    {
        Invalid = -1,
        Normal,
        Boost,
    };

    struct DisplayVersion {
        char name[16];
    };

    void Initialize();
    void SetPerformanceConfiguration(nn::oe::PerformanceMode, s32);
    s32 GetOperationMode();
    s32 GetPerformanceMode();
    void SetResumeNotificationEnabled(bool);
    void SetOperationModeChangedNotificationEnabled(bool);
    void SetPerformanceModeChangedNotificationEnabled(bool);
    void SetFocusHandlingMode(nn::oe::FocusHandlingMode);
    bool TryPopNotificationMessage(u32*);
    s32 GetCurrentFocusState();
    void EnableGamePlayRecording(void*, u64);
    bool IsUserInactivityDetectionTimeExtended();
    void SetUserInactivityDetectionTimeExtended(bool);
    void FinishStartupLogo();
    nn::settings::LanguageCode GetDesiredLanguage();
    void GetDisplayVersion(DisplayVersion*);
    void SetCpuBoostMode(nn::oe::CpuBoostMode);
};  // namespace oe
};  // namespace nn