#include <stdlib.h>

#include "font.h"
#include "freetype/freetype.h"
#include "ifor.h"
#include "renderer.h"
#include "wayland.h"

int main(void)
{
    IFOR_state state = {0};
    Renderer renderer = {0};
    init_state(&state, &renderer, 900, 400);

    if (!wayland_init(&state)) {
        return EXIT_FAILURE;
    }

    FT_Library library;
    FT_Face face;

    if (!freetype_init(&library, &face)) {
        return EXIT_FAILURE;
    }

    wayland_main_loop(&state);

    wayland_cleanup(&state);
    renderer_cleanup(state.renderer);
    FT_Done_FreeType(library);

    return EXIT_SUCCESS;
}
