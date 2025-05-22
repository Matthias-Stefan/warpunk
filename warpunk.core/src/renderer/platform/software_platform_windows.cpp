#include "warpunk.core/src/defines.h"
#if defined(WARPUNK_WINDOWS)

#include "warpunk.core/src/renderer/platform/software_platform.h"

b8 software_platform_startup()
{  
    return true;
}

b8 software_platform_submit_framebuffer(s32 width, s32 height, s32 size, u8* framebuffer)
{
    return true;
}

#endif // WARPUNK_LINUX
