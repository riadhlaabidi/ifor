#include "ifor.h"
#include "renderer.h"

void init_state(IFOR_state *state, Renderer *renderer, uint32_t w_width,
                uint32_t w_height)
{
    state->surface_width = w_width;
    state->surface_height = w_height;
    state->renderer = renderer;
    state->quit = 0;
}
