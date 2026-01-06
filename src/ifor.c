#include <GLES3/gl3.h>
#include <stdint.h>
#include <string.h>
#include <wayland-egl.h>

#include "config.h"
#include "font.h"
#include "ifor.h"
#include "la.h"
#include "renderer.h"

void state_init(IFOR_state *state, Renderer *renderer, Atlas *atlas,
                uint32_t w_width, uint32_t w_height)
{
    state->surface_width = w_width;
    state->surface_height = w_height;
    state->renderer = renderer;
    state->atlas = atlas;
    state->quit = 0;
}

static void render_apps_list(IFOR_state *state, const char **apps)
{
    float padding = 64.0f;
    float padding_x = (padding / 1920.0f) * state->surface_width;
    float padding_y = (padding / 1080.0f) * state->surface_height;
    float vertical_padding = 24.0f;

    Vec2f origin = vec2f(0, state->surface_height);
    Vec2f pen_position =
        vec2f_add(origin, vec2f(padding_x, -padding_y - state->atlas->height));

    for (int i = 0; i < 4; i++) {
        if (i == state->selected) {
            Vec2f pen_pos = vec2f(0, pen_position.y + state->atlas->height +
                                         vertical_padding / 2.0f);
            Vec2f rect_size = vec2f(state->surface_width,
                                    -vertical_padding - state->atlas->height);
            glUseProgram(state->renderer->programs[COLOR_SHADER]);
            renderer_solid_rectanlge(state->renderer, pen_pos, rect_size,
                                     vec4fu(0.8f));
            renderer_flush(state->renderer);
        }
        glUseProgram(state->renderer->programs[TEXT_SHADER]);
        freetype_render_text(state->atlas, state->renderer, apps[i],
                             strlen(apps[i]), pen_position, vec4fu(1.0f));
        renderer_flush(state->renderer);
        pen_position = vec2f_add(
            pen_position, vec2f(0, -(state->atlas->height + vertical_padding)));
    }
}

void render(IFOR_state *state)
{
    const char *apps[4] = {
        "Firefox (web browser)",
        "VLC (media player)",
        "Eye of gnome (Image viewer)",
        "Ghostty (Terminal)",
    };

    glClearColor(0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    render_apps_list(state, apps);
}
