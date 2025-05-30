#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wayland-client.h>

#include "font.h"
#include "ifor.h"
#include "wayland.h"
#include "wlr-layer-shell-unstable-v1-client-protocol.h"

static int create_shm_file(size_t size)
{
    char template[] = "/tmp/wayland-shm-XXXXXX";
    int fd = mkstemp(template);
    if (fd < 0) {
        return -1;
    }
    unlink(template);

    if (ftruncate(fd, size) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static void release_buffer(void *data, struct wl_buffer *wl_buffer)
{
    (void)data;
    wl_buffer_destroy(wl_buffer);
}

static const struct wl_buffer_listener buffer_listener = {
    .release = release_buffer};

static struct wl_buffer *draw_frame(IFOR_state *state)
{
    int width = state->surface_width;
    int height = state->surface_height;

    int stride = width * 4; // ARGB8888 4 bytes format
    int size = stride * height;
    int fd = create_shm_file(size);

    if (fd < 0) {
        fprintf(stderr, "Failed to create shm file.\n");
        return NULL;
    }

    uint32_t *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
                          0);
    if (data == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(state->shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(
        pool, 0, width, height, stride, WL_SHM_FORMAT_ABGR8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    for (int i = 0; i < width * height; i++) {
        data[i] = 0xDD000000;
    }

    render(data, width, "Hello, World!", 13);

    munmap(data, size);
    wl_buffer_add_listener(buffer, &buffer_listener, NULL);
    return buffer;
}

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *layer_surface,
                                    uint32_t serial, uint32_t width,
                                    uint32_t height)
{
    (void)width;
    (void)height;

    IFOR_state *state = data;
    zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

    struct wl_buffer *buffer = draw_frame(state);
    wl_surface_attach(state->surface, buffer, 0, 0);
    wl_surface_damage(state->surface, 0, 0, state->surface_width,
                      state->surface_height);
    wl_surface_commit(state->surface);
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

static void output_handle_mode(void *data, struct wl_output *wl_output,
                               uint32_t flags, int32_t width, int32_t height,
                               int32_t refresh)
{
    (void)wl_output;
    (void)refresh;

    IFOR_state *state = data;

    if (flags & WL_OUTPUT_MODE_CURRENT) {
        state->screen_width = width;
        state->screen_height = height;
    }
}

static void no_op() {}

static const struct wl_output_listener output_listener = {
    .geometry = no_op,
    .mode = output_handle_mode,
    .name = no_op,
    .description = no_op,
    .done = no_op,
    .scale = no_op,
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
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        state->shm = wl_registry_bind(wl_registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        state->layer_shell = wl_registry_bind(
            wl_registry, name, &zwlr_layer_shell_v1_interface, 4);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        state->output = wl_registry_bind(wl_registry, name,
                                         &wl_output_interface, 4);
        wl_output_add_listener(state->output, &output_listener, state);
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
    if (!freetype_init()) {
        return 0;
    }

    state->display = wl_display_connect(NULL);

    if (!state->display) {
        fprintf(stderr, "Failed to connect to wayland display.\n");
        return 0;
    }

    state->registry = wl_display_get_registry(state->display);
    wl_registry_add_listener(state->registry, &registry_listener, state);
    wl_display_roundtrip(state->display);
    wl_display_roundtrip(state->display);

    state->surface = wl_compositor_create_surface(state->compositor);
    state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        state->layer_shell, state->surface, NULL, 3, "overlay");

    zwlr_layer_surface_v1_add_listener(state->layer_surface,
                                       &layer_surface_listener, state);
    zwlr_layer_surface_v1_set_anchor(state->layer_surface, 0);
    zwlr_layer_surface_v1_set_exclusive_zone(state->layer_surface, -1);

    int margin_top = (state->screen_height - state->surface_height) / 2;
    int margin_left = (state->screen_width - state->surface_width) / 2;
    zwlr_layer_surface_v1_set_margin(state->layer_surface, margin_top, 0, 0,
                                     margin_left);

    zwlr_layer_surface_v1_set_keyboard_interactivity(
        state->layer_surface, /* Until I implement the quit shortcut */
        ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND);
    wl_surface_commit(state->surface);

    // struct wl_callback *callback = wl_surface_frame(state.surface);
    // wl_callback_add_listener(callback, &surface_frame_listener, &state);

    while (wl_display_dispatch(state->display)) {
    }

    return 1;
}

void wayland_cleanup(IFOR_state *state)
{
    freetype_cleanup();
    wl_output_destroy(state->output);
    zwlr_layer_surface_v1_destroy(state->layer_surface);
    wl_surface_destroy(state->surface);
    zwlr_layer_shell_v1_destroy(state->layer_shell);
    wl_compositor_destroy(state->compositor);
    wl_shm_destroy(state->shm);
    wl_registry_destroy(state->registry);
    wl_display_disconnect(state->display);
}
