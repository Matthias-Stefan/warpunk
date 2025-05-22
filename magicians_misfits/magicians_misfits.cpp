#include <warpunk.core/src/defines.h>
#include <warpunk.runtime/src/entry.h>

b8 boot(application* app)
{
    return true;
}

b8 update(application* app)
{
    return true;
}

b8 prepare_frame(application* app)
{
    return true;
}

b8 render_frame(application* app)
{
    return true;
}

b8 shutdown(application* app)
{
    return true;
}

b8 application_create(application* out_app)
{
    out_app->boot = boot;
    out_app->update = update;
    out_app->prepare_frame = prepare_frame;
    out_app->render_frame = render_frame;
    out_app->shutdown = shutdown;
    return true;
}

b8 application_initialize(application* app)
{
    return true;
}