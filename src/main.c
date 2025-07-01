#include <stdio.h>
#include <stdlib.h>

#include "font.h"
#include "freetype/freetype.h"
#include "ifor.h"
#include "renderer.h"
#include "wayland.h"

int main(void)
{
    Renderer renderer = {0};
    IFOR_state state = {0};
    state.renderer = &renderer;
    state.surface_width = 900;
    state.surface_height = 400;

    FT_Library library;
    FT_Face face;

    if (!freetype_init(&library, &face)) {
        return EXIT_FAILURE;
    }

    // if (!renderer_init(state.renderer)) {
    //     return EXIT_FAILURE;
    // }

    if (!wayland_init(state.renderer, &state)) {
        return EXIT_FAILURE;
    }

    wayland_cleanup(&state);
    renderer_cleanup(state.renderer);
    FT_Done_FreeType(library);

    return EXIT_SUCCESS;
}
