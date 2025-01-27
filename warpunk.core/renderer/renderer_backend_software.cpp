#include "warpunk.core/renderer/renderer_backend.h"

#include "warpunk.core/defines.h"
#include "warpunk.core/renderer/platform/software_platform.h"
#include "warpunk.core/math/v3.hpp"

u8 framebuffer[1920 * 1080 * 4];

[[nodiscard]] b8 renderer_startup(renderer_config_t renderer_config)
{
    if (!software_platform_startup())
    {
        return false;
    }

    return true;
}   

void renderer_begin_frame()
{
    static s32 offset_x = 0;
    static s32 offset_y = 0;

    u8* row = (u8 *)framebuffer;
    for(u16 y = 0; y < 1080; ++y) 
    {
        u32* pixel = (u32*)row;
        for(u16 x = 0; x < 1920; ++x) 
        {
            u8 alpha = 255;
            u8 red = (u8)(static_cast<f64>(x) / offset_x * 255.999);    
            u8 green = (u8)(static_cast<f64>(y) / offset_y * 255.999);   
            u8 blue  = 0;
            *pixel++ =((((alpha << 24) | red << 16) | green << 8) | blue);
        }

         row += 1920 * 4;
    }

    [[maybe_unused]] bool _ = software_platform_submit_framebuffer(1920, 1080, 1920*1080*4, framebuffer);

    offset_x += 1;
    if (offset_x % 1919 == 0)
    {
        offset_x = 0;
    }
    offset_y += 1;
    if (offset_y % 1079 == 0)
    {
        offset_y = 0;
    }

}
