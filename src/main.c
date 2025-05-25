#include <stdlib.h>

#include "ifor.h"
#include "wayland.h"

int main(void)
{
    IFOR_state state = {0};
    state.surface_width = 800;
    state.surface_height = 300;

    if (!wayland_init(&state)) {
        return EXIT_FAILURE;
    }

    wayland_cleanup(&state);

    return EXIT_SUCCESS;
}
