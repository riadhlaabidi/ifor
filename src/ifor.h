#ifndef IFOR_IFOR_H
#define IFOR_IFOR_H

#include <EGL/egl.h>
#include <stdint.h>
#include <wayland-client.h>

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

typedef struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_surface *surface;

    struct zwlr_layer_shell_v1 *layer_shell;
    struct zwlr_layer_surface_v1 *layer_surface;

    struct egl_dispay *egl_display;
    struct wl_egl_window *egl_window;
    struct EGLSurface *egl_surface;
    struct EGLContext *egl_context;

    int32_t surface_width;
    int32_t surface_height;
    // uint32_t last_frame;
} IFOR_state;

#endif /* end of include guard: IFOR_IFOR_H */
