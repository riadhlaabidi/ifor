#include "ifor.h"
#include "la.h"
#include "renderer.h"
#include <GLES3/gl3.h>
#include <stddef.h>

void init_state(IFOR_state *state, Renderer *renderer, uint32_t w_width,
                uint32_t w_height)
{
    state->surface_width = w_width;
    state->surface_height = w_height;
    state->renderer = renderer;
    state->quit = 0;
}

void render(IFOR_state *state)
{
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(state->renderer->programs[TEXT_SHADER]);
    Vec2f pos = vec2f(0, (float)state->surface_height - state->atlas->height);
    freetype_render_text(state->atlas, state->renderer, "Hello world!", 12, pos,
                         vec4fu(1));
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    state->renderer->vertices_count * sizeof(Vertex),
                    state->renderer->vertices);
    glDrawArrays(GL_TRIANGLES, 0, state->renderer->vertices_count);
    state->renderer->vertices_count = 0;
}
