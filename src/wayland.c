#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>

#include "renderer.h"
#include "wayland.h"

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height)
{
    (void)width;
    (void)height;

    IFOR_state *state = data;
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
    if (width && height) {
        wl_egl_window_resize(state->egl_window, width, height, 0, 0);
    }
    render(state);
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

int wayland_init(IFOR_state *state)
{
    state->display = wl_display_connect(NULL);

    if (!state->display) {
        fprintf(stderr, "Failed to connect to wayland display.\n");
        return 0;
    }

    state->registry = wl_display_get_registry(state->display);
    wl_registry_add_listener(state->registry, &registry_listener, state);
    wl_display_roundtrip(state->display);

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
        EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,
        EGL_RED_SIZE,
        8,
        EGL_GREEN_SIZE,
        8,
        EGL_BLUE_SIZE,
        8,
        EGL_ALPHA_SIZE,
        8,
        EGL_NONE,
    };

    EGLConfig config;
    EGLint num_config;
    eglChooseConfig(state->egl_display, config_attribs, &config, 1,
                    &num_config);

    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION,
        2,
        EGL_NONE,
    };
    state->egl_context = eglCreateContext(state->egl_display, config,
                                          EGL_NO_CONTEXT, context_attribs);

    state->surface = wl_compositor_create_surface(state->compositor);
    state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        state->layer_shell, state->surface, NULL, 3, "Ifor launcher");

    zwlr_layer_surface_v1_add_listener(state->layer_surface,
                                       &layer_surface_listener, state);
    zwlr_layer_surface_v1_set_size(state->layer_surface, state->surface_width,
                                   state->surface_height);

    zwlr_layer_surface_v1_set_keyboard_interactivity(
        state->layer_surface, /* Until I implement the quit shortcut */
        ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND);

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

    if (!gl_init()) {
        return 0;
    }

    wl_surface_commit(state->surface);

    // struct wl_callback *callback = wl_surface_frame(state.surface);
    // wl_callback_add_listener(callback, &surface_frame_listener, &state);

    while (wl_display_dispatch(state->display)) {
    }

    return 1;
}

void wayland_cleanup(IFOR_state *state)
{
    gl_cleanup();
    eglDestroySurface(state->egl_display, state->egl_surface);
    eglDestroyContext(state->egl_display, state->egl_context);
    wl_egl_window_destroy(state->egl_window);
    eglTerminate(state->display);

    zwlr_layer_surface_v1_destroy(state->layer_surface);
    zwlr_layer_shell_v1_destroy(state->layer_shell);

    wl_surface_destroy(state->surface);
    wl_compositor_destroy(state->compositor);
    wl_registry_destroy(state->registry);
    wl_display_disconnect(state->display);
}
