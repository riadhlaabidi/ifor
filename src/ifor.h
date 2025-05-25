#ifndef IFOR_IFOR_H
#define IFOR_IFOR_H

#include <wayland-client.h>

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

typedef struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_shm *shm;
    struct wl_compositor *compositor;
    struct zwlr_layer_shell_v1 *layer_shell;
    struct wl_surface *surface;
    struct zwlr_layer_surface_v1 *layer_surface;
    // uint32_t last_frame;
} IFOR_state;

#endif /* end of include guard: IFOR_IFOR_H */
