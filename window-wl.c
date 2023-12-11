#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include "window-wl.h"

struct wl_display *display = NULL;
struct wl_compositor *compositor = NULL;
struct wl_surface *surface;
struct wl_shell *shell;
struct wl_shell_surface *shell_surface;
struct wl_shm *shm;
struct wl_buffer *buffer;
struct wl_callback *frame_callback;
struct wl_seat *seat;
struct wl_keyboard *keyboard;

struct Image {
    int width;
    int height;
    int depth;
    int stride;
    int size;
    int* data;
};

struct Image* image;

static int WIDTH;
static int HEIGHT;
void (*paintPixels)();
void (*keyChange)(unsigned int key, unsigned int state);

static void
handle_ping(void *data, struct wl_shell_surface *shell_surface,
							uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
    fprintf(stderr, "Pinged and ponged\n");
}

static void
handle_configure(void *data, struct wl_shell_surface *shell_surface,
		 uint32_t edges, int32_t width, int32_t height)
{
}

static void
handle_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

static const struct wl_shell_surface_listener shell_surface_listener = {
	handle_ping,
	handle_configure,
	handle_popup_done
};

static int
set_cloexec_or_close(int fd)
{
        long flags;

        if (fd == -1)
                return -1;

        flags = fcntl(fd, F_GETFD);
        if (flags == -1)
                goto err;

        if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
                goto err;

        return fd;

err:
        close(fd);
        return -1;
}

static int
create_tmpfile_cloexec(char *tmpname)
{
        int fd;

#ifdef HAVE_MKOSTEMP
        fd = mkostemp(tmpname, O_CLOEXEC);
        if (fd >= 0)
                unlink(tmpname);
#else
        fd = mkstemp(tmpname);
        if (fd >= 0) {
                fd = set_cloexec_or_close(fd);
                unlink(tmpname);
        }
#endif

        return fd;
}

/*
 * Create a new, unique, anonymous file of the given size, and
 * return the file descriptor for it. The file descriptor is set
 * CLOEXEC. The file is immediately suitable for mmap()'ing
 * the given size at offset zero.
 *
 * The file should not have a permanent backing store like a disk,
 * but may have if XDG_RUNTIME_DIR is not properly implemented in OS.
 *
 * The file name is deleted from the file system.
 *
 * The file is suitable for buffer sharing between processes by
 * transmitting the file descriptor over Unix sockets using the
 * SCM_RIGHTS methods.
 */
int
os_create_anonymous_file(off_t size)
{
        static const char template[] = "/weston-shared-XXXXXX";
        const char *path;
        char *name;
        int fd;

        path = getenv("XDG_RUNTIME_DIR");
        if (!path) {
                errno = ENOENT;
                return -1;
        }

        name = malloc(strlen(path) + sizeof(template));
        if (!name)
                return -1;
        strcpy(name, path);
        strcat(name, template);

        fd = create_tmpfile_cloexec(name);

        free(name);

        if (fd < 0)
                return -1;

        if (ftruncate(fd, size) < 0) {
                close(fd);
                return -1;
        }

        return fd;
}

uint32_t pixel_value = 0x0; // black

static const struct wl_callback_listener frame_listener;

static void
redraw(void *data, struct wl_callback *callback, uint32_t time)
{
    // fprintf(stderr, "Redrawing\n");   
    wl_callback_destroy(frame_callback);
    wl_surface_damage(surface, 0, 0,
    		WIDTH, HEIGHT); 
    paintPixels();
    frame_callback = wl_surface_frame(surface);
    wl_surface_attach(surface, buffer, 0, 0);
    wl_callback_add_listener(frame_callback, &frame_listener, NULL);
    wl_surface_commit(surface);
}

static const struct wl_callback_listener frame_listener = {
    redraw
};

static struct wl_buffer *
create_buffer() {
    struct wl_shm_pool *pool;
    int stride = WIDTH * 4; // 4 bytes per pixel
    int size = stride * HEIGHT;
    int fd;
    struct wl_buffer *buff;

    image->width = WIDTH;
    image->height = HEIGHT;
    image->depth = 4;
    image->stride = stride;
    image->size = size;

    fd = os_create_anonymous_file(size);
    if (fd < 0) {
	fprintf(stderr, "creating a buffer file for %d B failed: %m\n",
		size);
	exit(1);
    }
    
    image->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (image->data == MAP_FAILED) {
	fprintf(stderr, "mmap failed: %m\n");
	close(fd);
	exit(1);
    }

    pool = wl_shm_create_pool(shm, fd, size);
    buff = wl_shm_pool_create_buffer(pool, 0,
					  WIDTH, HEIGHT,
					  stride, 	
					  WL_SHM_FORMAT_XRGB8888);
    //wl_buffer_add_listener(buffer, &buffer_listener, buffer);
    wl_shm_pool_destroy(pool);
    return buff;
}

static void
create_window() {

    buffer = create_buffer();

    wl_surface_attach(surface, buffer, 0, 0);
    //wl_surface_damage(surface, 0, 0, WIDTH, HEIGHT);
    wl_surface_commit(surface);
}

static void
shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
    char *s;
    switch (format) {
    case WL_SHM_FORMAT_ARGB8888: s = "ARGB8888"; break;  
    case WL_SHM_FORMAT_XRGB8888: s = "XRGB8888"; break;  
    case WL_SHM_FORMAT_RGB565: s = "RGB565"; break;  
    default: s = "other format"; break;  
    }
    fprintf(stderr, "Possible shmem format %s\n", s);
}

struct wl_shm_listener shm_listener = {
	shm_format
};

static void
global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
	       const char *interface, uint32_t version)
{
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, 
				      id, 
				      &wl_compositor_interface, 
				      1);
    } else if (strcmp(interface, "wl_shell") == 0) {
        shell = wl_registry_bind(registry, id,
                                 &wl_shell_interface, 1);
    } else if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(registry, id,
                                 &wl_seat_interface, 1);
    } else if (strcmp(interface, "wl_shm") == 0) {
        shm = wl_registry_bind(registry, id,
                                 &wl_shm_interface, 1);
	wl_shm_add_listener(shm, &shm_listener, NULL);
       
    }
}

static void
global_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
    printf("Got a registry losing event for %d\n", id);
}

static const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover
};

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
    keyChange(key, state);
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

void initWindow(int width, int height, const char* title, void (*paint_pixels)(), void (*key_change)(unsigned int key, unsigned int state)) {
    paintPixels = paint_pixels;
    keyChange = key_change;
    WIDTH = width;
    HEIGHT = height;
    image = malloc(sizeof(struct Image));
    display = wl_display_connect(NULL);
    if (display == NULL) {
	fprintf(stderr, "Can't connect to display\n");
	exit(1);
    }
    printf("connected to display\n");

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);

    wl_display_dispatch(display);
    wl_display_roundtrip(display);

    if (compositor == NULL) {
	fprintf(stderr, "Can't find compositor\n");
	exit(1);
    } else {
	fprintf(stderr, "Found compositor\n");
    }

    

    surface = wl_compositor_create_surface(compositor);
    if (surface == NULL) {
	fprintf(stderr, "Can't create surface\n");
	exit(1);
    } else {
	fprintf(stderr, "Created surface\n");
    }

    shell_surface = wl_shell_get_shell_surface(shell, surface);
    if (shell_surface == NULL) {
	fprintf(stderr, "Can't create shell surface\n");
	exit(1);
    } else {
	fprintf(stderr, "Created shell surface\n");
    }
    wl_shell_surface_set_toplevel(shell_surface);

    wl_shell_surface_add_listener(shell_surface,
				  &shell_surface_listener, NULL);

    frame_callback = wl_surface_frame(surface);
    wl_callback_add_listener(frame_callback, &frame_listener, NULL);

keyboard = wl_seat_get_keyboard(seat);
    if (surface == NULL) {
	fprintf(stderr, "Can't get keyboard\n");
	exit(1);
    } else {
	fprintf(stderr, "Got keyboard\n");
    }
    wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);

    create_window();
    redraw(NULL, NULL, 0);
}

int dispatchEvents() {
    return wl_display_dispatch(display);
}

void closeWindow() {
    free(image);
    wl_display_disconnect(display);
    printf("disconnected from display\n");
}