#ifndef IFOR_IFOR_H
#define IFOR_IFOR_H

#include <EGL/egl.h>
#include <stdint.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "font.h"
#include "renderer.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#define ITEMS_PER_LIST 6

typedef struct {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_surface *surface;
    struct wl_seat *seat;
    struct wl_keyboard *keyboard;

    struct zwlr_layer_shell_v1 *layer_shell;
    struct zwlr_layer_surface_v1 *layer_surface;

    // xkb
    struct xkb_state *xkb_state;
    struct xkb_context *xkb_context;
    struct xkb_keymap *xkb_keymap;

    struct egl_dispay *egl_display;
    struct wl_egl_window *egl_window;
    struct EGLSurface *egl_surface;
    struct EGLContext *egl_context;

    uint32_t surface_width;
    uint32_t surface_height;
    // uint32_t last_frame;

    Renderer *renderer;
    Atlas *atlas;

    int quit;
    int surface_configured;
} IFOR_state;

void state_init(IFOR_state *state, Renderer *rederer, Atlas *atlas,
                uint32_t w_width, uint32_t w_height);

void render(IFOR_state *state);

#endif /* end of include guard: IFOR_IFOR_H */
