#include <stdlib.h>

#include "font.h"
#include "ifor.h"
#include "wayland.h"

int main(void)
{
    IFOR_state state = {0};
    state.surface_width = 900;
    state.surface_height = 400;

    if (!freetype_init()) {
        return EXIT_FAILURE;
    }

    if (!wayland_init(&state)) {
        return EXIT_FAILURE;
    }

    wayland_cleanup(&state);
    freetype_cleanup();

    return EXIT_SUCCESS;
}
