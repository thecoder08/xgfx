#include <string.h>
#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "window.h"
#include "window-xcb.h"

extern Image image;

xcb_connection_t* connection;
xcb_window_t windowXcb;
xcb_gcontext_t gc;
xcb_atom_t wmDeleteMessageXcb;
xcb_shm_seg_t segment;

int initWindow_xcb(int width, int height, const char* title) {
    connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(connection)) {
        fprintf(stderr, "Failed to connect to display!\n");
        return 1;
    }
    int shmFd = shm_open("xgfxShmFile", O_CREAT | O_RDWR, 0600);
    shm_unlink("xgfxShmFile");
    if (shmFd == -1) {
        fprintf(stderr, "Failed to create shm file\n");
        return 1;
    }
    image.size = width*height*4;
    if (ftruncate(shmFd, image.size) == -1) {
        fprintf(stderr, "Failed to set shm file size\n");
        return 1;
    }
    segment = xcb_generate_id(connection);
    xcb_shm_attach_fd(connection, segment, shmFd, 1);
    image.data = mmap(NULL, image.size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (image.data == NULL) {
        fprintf(stderr, "Failed to map framebuffer\n");
        return 1;
    }
    image.width = width;
    image.height = height;
    image.depth = 4;
    image.stride = width*4;

    xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    unsigned int mask = XCB_CW_EVENT_MASK;
    xcb_event_mask_t valwin[] = {XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE};
    windowXcb = xcb_generate_id(connection);
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, windowXcb, screen->root, 0, 0, width, height, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, valwin);
    gc = xcb_generate_id(connection);
    unsigned int values[] = {screen->black_pixel, screen->white_pixel};
    xcb_create_gc(connection, gc, windowXcb, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, values);
    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, windowXcb, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);
    xcb_map_window(connection, windowXcb);

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 1, 12,
"WM_PROTOCOLS");
  xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, cookie, 0);

  xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16,
"WM_DELETE_WINDOW");
  xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(connection, cookie2, 0);

  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, windowXcb, (*reply).atom, 4, 32, 1,
&(*reply2).atom);
    wmDeleteMessageXcb = reply2->atom;
    xcb_flush(connection);
    return 0;
}

int checkWindowEvent_xcb(Event* event) {
    xcb_generic_event_t* xcbevent;
    if ((xcbevent = xcb_poll_for_event(connection))) {
        switch(xcbevent->response_type & ~0x80) {
            case XCB_CLIENT_MESSAGE: {
            xcb_client_message_event_t* clientMessageEvent = (xcb_client_message_event_t*)xcbevent;
            if (clientMessageEvent->data.data32[0] == wmDeleteMessageXcb) {
                xcb_destroy_window(connection, windowXcb);
                xcb_disconnect(connection);
                event->type = WINDOW_CLOSE;
                free(xcbevent);
                return 1;
            };
            free(xcbevent);
            return 0;
            }
            case XCB_KEY_PRESS: {
            xcb_key_press_event_t* keypressEvent = (xcb_key_press_event_t*)xcbevent;
            event->type = KEY_CHANGE;
            event->keychange.state = 1;
            event->keychange.key = keypressEvent->detail - 8;
            free(xcbevent);
            return 1;
            }

            case XCB_KEY_RELEASE: {
            xcb_key_press_event_t* keyreleaseEvent = (xcb_key_press_event_t*)xcbevent;
            event->type = KEY_CHANGE;
            event->keychange.state = 0;
            event->keychange.key = keyreleaseEvent->detail - 8;
            free(xcbevent);
            return 1;
            }

            case XCB_MOTION_NOTIFY: {
            xcb_motion_notify_event_t* motionnotifyEvent = (xcb_motion_notify_event_t*)xcbevent;
            event->type = MOUSE_MOVE;
            event->mousemove.x = motionnotifyEvent->event_x;
            event->mousemove.y = motionnotifyEvent->event_y;
            free(xcbevent);
            return 1;
            }

            case XCB_BUTTON_PRESS: {
            xcb_button_press_event_t* buttonpressEvent = (xcb_button_press_event_t*)xcbevent;
            event->type = MOUSE_BUTTON;
            event->mousebutton.state = 1;
            event->mousebutton.button = buttonpressEvent->detail;
            free(xcbevent);
            return 1;
            }

            case XCB_BUTTON_RELEASE: {
            xcb_button_release_event_t* buttonreleaseEvent = (xcb_button_release_event_t*)xcbevent;
            event->type = MOUSE_BUTTON;
            event->mousebutton.state = 0;
            event->mousebutton.button = buttonreleaseEvent->detail;
            free(xcbevent);
            return 1;
            }

            default:
            free(xcbevent);
            return 0;
        }
    }
    return 0;
}

void updateWindow_xcb() {
    xcb_shm_put_image(connection, windowXcb, gc, image.width, image.height, 0, 0, image.width, image.height, 0, 0, 24, XCB_IMAGE_FORMAT_Z_PIXMAP, 0, segment, 0);
    xcb_flush(connection);
    usleep(10000);
}