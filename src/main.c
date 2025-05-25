#include <stdlib.h>

#include "ifor.h"
#include "wayland.h"

int main(void)
{
    IFOR_state state = {0};

    if (!wayland_init(&state)) {
        return EXIT_FAILURE;
    }

    wayland_cleanup(&state);

    return EXIT_SUCCESS;
}
