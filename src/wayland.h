#ifndef IFOR_WAYLAND_H
#define IFOR_WAYLAND_H

#include "ifor.h"

int wayland_init(IFOR_state *state);
int wayland_egl_init(IFOR_state *state);
void wayland_main_loop(IFOR_state *state);
void wayland_cleanup(IFOR_state *state);

#endif /* end of include guard: IFOR_WAYLAND_H */
