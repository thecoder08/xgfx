#include <wayland-client.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "xdg-shell.h"
#include "window.h"
#include "window-wl.h"

// these are static to avoid namespace collisions
static struct wl_display* display;
static struct wl_compositor* compositor;
static struct wl_surface* surface;
static struct wl_shm* shm;
static struct wl_buffer* buffer;
static struct xdg_wm_base* wm_base;
static struct wl_seat* seat;

extern Image image;

int eventIndex = 0;
int eventRead = 0;
Event eventBuffer[100];

int can_use_buffer = 1;

void global_handler(void* data, struct wl_registry* registry, unsigned int name, const char* interface, unsigned int version) {
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    }
    if (strcmp(interface, "wl_shm") == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    if (strcmp(interface, "xdg_wm_base") == 0) {
        wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
    if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    }
}

void global_remove_handler(void* data, struct wl_registry* registry, unsigned int name) {

}

struct wl_registry_listener registry_listener = {
    .global = global_handler,
    .global_remove = global_remove_handler
};

void wm_base_ping_handler(void* data, struct xdg_wm_base* wm_base, unsigned int serial) {
    xdg_wm_base_pong(wm_base, serial);
}

struct xdg_wm_base_listener wm_base_listener = {
    .ping = wm_base_ping_handler
};

void toplevel_configure_handler(void* data, struct xdg_toplevel* toplevel, int width, int height, struct wl_array* states) {

}

void toplevel_close_handler(void* data, struct xdg_toplevel* toplevel) {
    eventBuffer[eventIndex].type = WINDOW_CLOSE;
    eventIndex++;
}

void toplevel_configure_bounds_handler(void* data, struct xdg_toplevel* toplevel, int width, int height) {

}

struct xdg_toplevel_listener toplevel_listener = {
    .configure = toplevel_configure_handler,
    .close = toplevel_close_handler,
    .configure_bounds = toplevel_configure_bounds_handler
};

void surface_configure_handler(void* data, struct xdg_surface* shell_surface, unsigned int serial) {
    xdg_surface_ack_configure(shell_surface, serial);
}

struct xdg_surface_listener surface_listener = {
    .configure = surface_configure_handler
};

void buffer_release(void* data, struct wl_buffer* buffer) {
    can_use_buffer = 1;
}

struct wl_buffer_listener buffer_listener = {
    .release = buffer_release
};

/* input handling shit (copied from old window-wl.c) */

void keymap(void *data,
		       struct wl_keyboard *wl_keyboard,
		       uint32_t format,
		       int32_t fd,
		       uint32_t size) {}

	void enter(void *data,
		      struct wl_keyboard *wl_keyboard,
		      uint32_t serial,
		      struct wl_surface *surface,
		      struct wl_array *keys) {}
	
	void leave(void *data,
		      struct wl_keyboard *wl_keyboard,
		      uint32_t serial,
		      struct wl_surface *surface) {}
	
void key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    eventBuffer[eventIndex].type = KEY_CHANGE;
    eventBuffer[eventIndex].keychange.key = key;
    eventBuffer[eventIndex].keychange.state = state;
    eventIndex++;
}

	void modifiers(void *data,
			  struct wl_keyboard *wl_keyboard,
			  uint32_t serial,
			  uint32_t mods_depressed,
			  uint32_t mods_latched,
			  uint32_t mods_locked,
			  uint32_t group) {}

	void repeat_info(void *data,
			    struct wl_keyboard *wl_keyboard,
			    int32_t rate,
			    int32_t delay) {}



static const struct wl_keyboard_listener keyboard_listener = {
    keymap,
    enter,
    leave,
    key,
    modifiers,
    repeat_info
};

void pointer_enter(void *data,
		      struct wl_pointer *wl_pointer,
		      uint32_t serial,
		      struct wl_surface *surface,
		      wl_fixed_t surface_x,
		      wl_fixed_t surface_y) {}

void pointer_leave(void *data,
		      struct wl_pointer *wl_pointer,
		      uint32_t serial,
		      struct wl_surface *surface) {}

void pointer_motion(void *data,
		       struct wl_pointer *wl_pointer,
		       uint32_t time,
		       wl_fixed_t surface_x,
		       wl_fixed_t surface_y) {
    eventBuffer[eventIndex].type = MOUSE_MOVE;
    eventBuffer[eventIndex].mousemove.x = wl_fixed_to_int(surface_x);
    eventBuffer[eventIndex].mousemove.y = wl_fixed_to_int(surface_y);
    eventIndex++;
}

void pointer_button(void *data,
		       struct wl_pointer *wl_pointer,
		       uint32_t serial,
		       uint32_t time,
		       uint32_t button,
		       uint32_t state) {
                if (button == 272) {
                    eventBuffer[eventIndex].type = MOUSE_BUTTON;
                    eventBuffer[eventIndex].mousebutton.button = BUTTON_LEFT;
                    eventBuffer[eventIndex].mousebutton.state = state;
                    eventIndex++;
                }
                if (button == 273) {
                    eventBuffer[eventIndex].type = MOUSE_BUTTON;
                    eventBuffer[eventIndex].mousebutton.button = BUTTON_RIGHT;
                    eventBuffer[eventIndex].mousebutton.state = state;
                    eventIndex++;
                }
                if (button == 274) {
                    eventBuffer[eventIndex].type = MOUSE_BUTTON;
                    eventBuffer[eventIndex].mousebutton.button = BUTTON_MIDDLE;
                    eventBuffer[eventIndex].mousebutton.state = state;
                    eventIndex++;
                }
                if (button == 275) {
                    eventBuffer[eventIndex].type = MOUSE_BUTTON;
                    eventBuffer[eventIndex].mousebutton.button = BUTTON_BACKWARD;
                    eventBuffer[eventIndex].mousebutton.state = state;
                    eventIndex++;
                }
                if (button == 276) {
                    eventBuffer[eventIndex].type = MOUSE_BUTTON;
                    eventBuffer[eventIndex].mousebutton.button = BUTTON_FORWARD;
                    eventBuffer[eventIndex].mousebutton.state = state;
                    eventIndex++;
                }
}

void pointer_axis(void *data,
		     struct wl_pointer *wl_pointer,
		     uint32_t time,
		     uint32_t axis,
		     wl_fixed_t value) {
                if (axis == 0) {
                    if (wl_fixed_to_int(value) > 0) {
                        eventBuffer[eventIndex].type = MOUSE_BUTTON;
                        eventBuffer[eventIndex].mousebutton.button = BUTTON_SCROLL_DOWN;
                        eventBuffer[eventIndex].mousebutton.state = 1;
                        eventIndex++;
                        eventBuffer[eventIndex].type = MOUSE_BUTTON;
                        eventBuffer[eventIndex].mousebutton.button = BUTTON_SCROLL_DOWN;
                        eventBuffer[eventIndex].mousebutton.state = 0;
                        eventIndex++;
                    }
                    if (wl_fixed_to_int(value) < 0) {
                        eventBuffer[eventIndex].type = MOUSE_BUTTON;
                        eventBuffer[eventIndex].mousebutton.button = BUTTON_SCROLL_UP;
                        eventBuffer[eventIndex].mousebutton.state = 1;
                        eventIndex++;
                        eventBuffer[eventIndex].type = MOUSE_BUTTON;
                        eventBuffer[eventIndex].mousebutton.button = BUTTON_SCROLL_UP;
                        eventBuffer[eventIndex].mousebutton.state = 0;
                        eventIndex++;
                    }
                }
                if (axis == 1) {
                    if (wl_fixed_to_int(value) > 0) {
                        eventBuffer[eventIndex].type = MOUSE_BUTTON;
                        eventBuffer[eventIndex].mousebutton.button = BUTTON_SCROLL_LEFT;
                        eventBuffer[eventIndex].mousebutton.state = 1;
                        eventIndex++;
                        eventBuffer[eventIndex].type = MOUSE_BUTTON;
                        eventBuffer[eventIndex].mousebutton.button = BUTTON_SCROLL_LEFT;
                        eventBuffer[eventIndex].mousebutton.state = 0;
                        eventIndex++;
                    }
                    if (wl_fixed_to_int(value) < 0) {
                        eventBuffer[eventIndex].type = MOUSE_BUTTON;
                        eventBuffer[eventIndex].mousebutton.button = BUTTON_SCROLL_RIGHT;
                        eventBuffer[eventIndex].mousebutton.state = 1;
                        eventIndex++;
                        eventBuffer[eventIndex].type = MOUSE_BUTTON;
                        eventBuffer[eventIndex].mousebutton.button = BUTTON_SCROLL_RIGHT;
                        eventBuffer[eventIndex].mousebutton.state = 0;
                        eventIndex++;
                    }
                }
             }

void pointer_frame(void *data,
		      struct wl_pointer *wl_pointer) {}

void pointer_axis_source(void *data,
			    struct wl_pointer *wl_pointer,
			    uint32_t axis_source) {}

void pointer_axis_stop(void *data,
			  struct wl_pointer *wl_pointer,
			  uint32_t time,
			  uint32_t axis) {}

void pointer_axis_discrete(void *data,
			      struct wl_pointer *wl_pointer,
			      uint32_t axis,
			      int32_t discrete) {}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis,
    .frame = pointer_frame,
    .axis_source = pointer_axis_source,
    .axis_stop = pointer_axis_stop,
    .axis_discrete = pointer_axis_discrete
};

/* end of input handing shit */

int initWindow_wl(int width, int height, const char* title) {
    image.width = width;
    image.height = height;
    image.depth = 4;
    image.stride = image.width*image.depth;
    image.size = image.stride*image.height;

    display = wl_display_connect(NULL);
    if (display == NULL) {
        fprintf(stderr, "Failed to connect to display!\n");
        return 1;
    }
    printf("Connected to display\n");

    struct wl_registry* registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);

    wl_display_roundtrip(display);
    if (compositor == NULL) {
        fprintf(stderr, "Failed to bind to compositor\n");
        return 1;
    }
    if (shm == NULL) {
        fprintf(stderr, "Failed to bind to shm\n");
        return 1;
    }
    if (wm_base == NULL) {
        fprintf(stderr, "Failed to bind to shell\n");
        return 1;
    }
    if (seat == NULL) {
        fprintf(stderr, "Failed to bind to seat\n");
        return 1;
    }
    printf("Binded to all globals\n");

    surface = wl_compositor_create_surface(compositor); // create a surface that we can attach/commit buffers to
    xdg_wm_base_add_listener(wm_base, &wm_base_listener, NULL);
    struct xdg_surface* shell_surface = xdg_wm_base_get_xdg_surface(wm_base, surface); // get shell_surface for surface
    xdg_surface_add_listener(shell_surface, &surface_listener, NULL);
    struct xdg_toplevel* toplevel = xdg_surface_get_toplevel(shell_surface); // assign toplevel role
    xdg_toplevel_set_title(toplevel, title);
    xdg_toplevel_add_listener(toplevel, &toplevel_listener, NULL);


    int shmFd = shm_open("waylandShmFile", O_CREAT | O_RDWR, 0600);
    shm_unlink("waylandShmFile");
    if (shmFd == -1) {
        fprintf(stderr, "Failed to create shm file\n");
        return 1;
    }
    if (ftruncate(shmFd, image.size) == -1) {
        fprintf(stderr, "Failed to set shm file size\n");
        return 1;
    }

    struct wl_shm_pool* shm_pool = wl_shm_create_pool(shm, shmFd, image.size);
    buffer = wl_shm_pool_create_buffer(shm_pool, 0, image.width, image.height, image.stride, WL_SHM_FORMAT_XRGB8888);
    wl_buffer_add_listener(buffer, &buffer_listener, NULL);

    image.data = mmap(NULL, image.size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (image.data == NULL) {
        fprintf(stderr, "Failed to map framebuffer\n");
        return 1;
    }

    struct wl_keyboard* keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    struct wl_pointer* pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, NULL);

    wl_surface_commit(surface); // needed for xdg_surface configure
    wl_display_roundtrip(display);

    printf("Initialized\n");
    return 0;
}

int checkWindowEvent_wl(Event* event) {
    if (eventIndex > eventRead) {
        *event = eventBuffer[eventRead];
        eventRead++;
        return 1;
    }
    return 0;
}

void updateWindow_wl() {
    eventIndex = 0;
    eventRead = 0;
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage_buffer(surface, 0, 0, 640, 480);
    wl_surface_commit(surface);
    can_use_buffer = 0; // we can't modify the buffer again until we recieve a release event
    while(!can_use_buffer) {
        wl_display_dispatch(display);
    }
    usleep(10000);
}