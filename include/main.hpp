#pragma once

#include <map>
#include <string>

#include "ModuleObject.hpp"
#include "alloc.h"
#include "mem.h"
#include "nn/crypto.h"
#include "nn/diag.h"
#include "nn/err.h"
#include "nn/fs.h"
#include "nn/nn.h"
#include "nn/oe.h"
#include "nn/os.hpp"
#include "nn/prepo.h"
#include "nn/ro.h"
#include "nvn/pfnc.h"
#include "operator.h"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "skyline/nx/kernel/virtmem.h"
#include "skyline/nx/runtime/env.h"

#ifdef __cplusplus
};
#endif

extern nn::os::EventType romMountedEvent;

extern "C" void skyline_init();