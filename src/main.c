#include "font.h"
#include "ifor.h"
#include "renderer.h"
#include "wayland.h"

int main(void)
{
    IFOR_state state = {0};
    Renderer renderer = {0};
    Atlas atlas = {0};

    state_init(&state, &renderer, &atlas, 900, 400);

    FT_Library library;
    FT_Face face;

    if (!freetype_init(&library, &face)) {
        return EXIT_FAILURE;
    }

    if (!wayland_init(&state)) {
        return EXIT_FAILURE;
    }

    if (!wayland_egl_init(&state)) {
        return EXIT_FAILURE;
    }

    if (!renderer_init(&renderer, state.surface_width, state.surface_height)) {
        return EXIT_FAILURE;
    }

    if (!freetype_create_texture_atlas(&atlas, face)) {
        return EXIT_FAILURE;
    }

    wayland_main_loop(&state);

    wayland_cleanup(&state);
    renderer_cleanup(&renderer);
    freetype_cleanup(library, face);

    return EXIT_SUCCESS;
}
