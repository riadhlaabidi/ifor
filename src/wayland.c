#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-egl.h>
#include <xkbcommon/xkbcommon.h>

#include "renderer.h"
#include "wayland.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height)
{
    IFOR_state *state = data;
    if (state->surface_configured) {
        return;
    }
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
    if (width && height) {
        wl_egl_window_resize(state->egl_window, width, height, 0, 0);
    }
    render(state);
    eglSwapBuffers(state->egl_display, state->egl_surface);
    state->surface_configured = 1;
}

static void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *layer_surface)
{
    (void)layer_surface;

    IFOR_state *state = data;
    zwlr_layer_surface_v1_destroy(state->layer_surface);
    state->layer_surface = NULL;

    wl_surface_destroy(state->surface);
    state->surface = NULL;
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t format, int32_t fd, uint32_t size)
{
    (void)wl_keyboard;

    IFOR_state *state = data;
    assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

    char *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    assert(map_shm != MAP_FAILED);

    struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
        state->xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);

    munmap(map_shm, size);
    close(fd);

    struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
    xkb_keymap_unref(state->xkb_keymap);
    xkb_state_unref(state->xkb_state);
    state->xkb_keymap = xkb_keymap;
    state->xkb_state = xkb_state;
}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t serial, struct wl_surface *surface,
                              struct wl_array *keys)
{
    (void)data;
    (void)wl_keyboard;
    (void)serial;
    (void)surface;
    (void)keys;

    /* NOTE: I should ignore these for the moment */

    // uint32_t *key;
    // wl_array_for_each(key, keys) {
    //     char buf[128];
    //     xkb_keysym_t sym = xkb_state_key_get_one_sym(state->xkb_state,
    //                                                  *key + 8);
    //     xkb_keysym_get_name(sym, buf, sizeof(buf));
    //     fprintf(stderr, "sym: %-12s (%d), ", buf, sym);
    //     xkb_state_key_get_utf8(state->xkb_state, *key + 8, buf, sizeof(buf));
    //     fprintf(stderr, "utf8:  '%s'\n", buf);
    // }
}
static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t serial, struct wl_surface *surface)
{
    (void)data;
    (void)wl_keyboard;
    (void)serial;
    (void)surface;
}

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                            uint32_t serial, uint32_t time, uint32_t key,
                            uint32_t key_state)
{
    (void)wl_keyboard;
    (void)serial;
    (void)time;
    (void)key_state;

    IFOR_state *state = data;
    uint32_t keycode = key + 8;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(state->xkb_state, keycode);
    if (sym == XKB_KEY_Escape) {
        state->quit = 1;
    } else {
        // char buf[128];
        // xkb_keysym_get_name(sym, buf, sizeof(buf));
        // const char *action = WL_KEYBOARD_KEY_STATE_PRESSED ? "pressed"
        //                                                    : "released";
        // fprintf(stderr, "key %s: sym: %-12s (%d), ", action, buf, sym);
        // xkb_keysym_to_utf8(sym, buf, sizeof(buf));
        // fprintf(stderr, "utf8: '%s'\n", buf);
    }
}
static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                                  uint32_t serial, uint32_t mods_depressed,
                                  uint32_t mods_latched, uint32_t mods_locked,
                                  uint32_t group)
{
    (void)wl_keyboard;
    (void)serial;
    IFOR_state *state = data;
    xkb_state_update_mask(state->xkb_state, mods_depressed, mods_latched,
                          mods_locked, 0, 0, group);
}
static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                    int32_t rate, int32_t delay)
{
    (void)data;
    (void)wl_keyboard;
    (void)rate;
    (void)delay;
    /* TODO: repeating text input and keyboard shortcuts? timing? */
}

static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};

static void wl_seat_name(void *data, struct wl_seat *wl_seat, const char *name)
{
    (void)data;
    (void)wl_seat;
    (void)name;
    /* TODO: log */
}

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
                                 uint32_t capabilities)
{
    IFOR_state *state = data;
    int have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

    if (have_keyboard && state->keyboard == NULL) {
        state->keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(state->keyboard, &wl_keyboard_listener, state);
    } else if (!have_keyboard && state->keyboard) {
        wl_keyboard_release(state->keyboard);
        state->keyboard = NULL;
    }
}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};

static void registry_global(void *data, struct wl_registry *wl_registry,
                            uint32_t name, const char *interface,
                            uint32_t version)
{
    (void)version;

    IFOR_state *state = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state->compositor = wl_registry_bind(wl_registry, name,
                                             &wl_compositor_interface, 4);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        state->layer_shell = wl_registry_bind(
            wl_registry, name, &zwlr_layer_shell_v1_interface, 4);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        state->seat = wl_registry_bind(wl_registry, name, &wl_seat_interface,
                                       7);
        wl_seat_add_listener(state->seat, &wl_seat_listener, state);
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name)
{
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static int wayland_egl_init(IFOR_state *state)
{
    state->egl_display = eglGetDisplay(state->display);

    if (!eglInitialize(state->egl_display, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize egl\n");
        return 0;
    }

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        fprintf(stderr, "Failed to bind EGL_OPENGL_ES_API\n");
        return 0;
    }

    EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,
        EGL_RED_SIZE,
        1,
        EGL_GREEN_SIZE,
        1,
        EGL_BLUE_SIZE,
        1,
        EGL_ALPHA_SIZE,
        1,
        EGL_NONE,
    };

    EGLConfig config;
    EGLint num_config;
    eglChooseConfig(state->egl_display, config_attribs, &config, 1,
                    &num_config);

    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        3,
        EGL_CONTEXT_MAJOR_VERSION,
        3,
        EGL_CONTEXT_MINOR_VERSION,
        2,
        EGL_NONE,
    };
    state->egl_context = eglCreateContext(state->egl_display, config,
                                          EGL_NO_CONTEXT, context_attribs);

    state->egl_window = wl_egl_window_create(
        state->surface, state->surface_width, state->surface_height);
    state->egl_surface =
        eglCreateWindowSurface(state->egl_display, config,
                               (EGLNativeWindowType)state->egl_window, NULL);

    if (!eglMakeCurrent(state->egl_display, state->egl_surface,
                        state->egl_surface, state->egl_context)) {
        fprintf(stderr, "Failed to make egl context current\n");
        return 0;
    }

    return 1;
}

int wayland_init(IFOR_state *state)
{
    state->display = wl_display_connect(NULL);

    if (!state->display) {
        fprintf(stderr, "Failed to connect to wayland display.\n");
        return 0;
    }

    state->registry = wl_display_get_registry(state->display);
    state->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    wl_registry_add_listener(state->registry, &registry_listener, state);
    wl_display_roundtrip(state->display);

    state->surface = wl_compositor_create_surface(state->compositor);
    state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        state->layer_shell, state->surface, NULL, 3,
        "Ifor application launcher");

    zwlr_layer_surface_v1_add_listener(state->layer_surface,
                                       &layer_surface_listener, state);
    zwlr_layer_surface_v1_set_size(state->layer_surface, state->surface_width,
                                   state->surface_height);

    zwlr_layer_surface_v1_set_keyboard_interactivity(
        state->layer_surface,
        ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);

    if (!wayland_egl_init(state)) {
        return 0;
    }

    if (!renderer_init(state->renderer, state->surface_width,
                       state->surface_height)) {
        return 0;
    }

    wl_surface_commit(state->surface);
    // struct wl_callback *callback = wl_surface_frame(state.surface);
    // wl_callback_add_listener(callback, &surface_frame_listener, &state);

    return 1;
}

void wayland_main_loop(IFOR_state *state)
{

    while (!state->quit && wl_display_dispatch(state->display)) {
    }
}

void wayland_cleanup(IFOR_state *state)
{
    eglDestroySurface(state->egl_display, state->egl_surface);
    eglDestroyContext(state->egl_display, state->egl_context);
    wl_egl_window_destroy(state->egl_window);
    eglTerminate(state->display);

    zwlr_layer_surface_v1_destroy(state->layer_surface);
    zwlr_layer_shell_v1_destroy(state->layer_shell);

    xkb_keymap_unref(state->xkb_keymap);
    xkb_context_unref(state->xkb_context);
    xkb_state_unref(state->xkb_state);

    wl_keyboard_destroy(state->keyboard);
    wl_seat_destroy(state->seat);
    wl_surface_destroy(state->surface);
    wl_compositor_destroy(state->compositor);
    wl_registry_destroy(state->registry);
    wl_display_disconnect(state->display);
}
