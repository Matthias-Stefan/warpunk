#pragma once

#include "warpunk.core/src/defines.h"

typedef enum _renderer_config_flag_bits_e
{
    RENDERER_CONFIG_FLAG_VSYNC_ENABLED_BIT = 0x1,
    RENDERER_CONFIG_FLAG_POWER_SAVING_BIT = 0x2,
    RENDERER_CONFIG_FLAG_ENABLE_VALIDATION = 0x4,
} renderer_config_flag_bits_e;

typedef u32 renderer_config_flag;

