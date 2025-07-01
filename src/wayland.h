#ifndef IFOR_WAYLAND_H
#define IFOR_WAYLAND_H

#include "ifor.h"
#include "renderer.h"

int wayland_init(Renderer *renderer, IFOR_state *state);
void wayland_cleanup(IFOR_state *state);

#endif /* end of include guard: IFOR_WAYLAND_H */
