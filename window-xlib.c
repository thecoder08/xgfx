#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "window.h"
#include "window-xlib.h"

extern Image image;

Display* display;
Window window;
int screen;
XImage* ximage;
Atom wmDeleteMessage;

int initWindow_xlib(int width, int height, const char* title) {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
      fprintf(stderr, "Failed to connect to display!\n");
      return 1;
    }
    screen = DefaultScreen(display);
    window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, width, height, 0, BlackPixel(display, screen), WhitePixel(display, screen));
    XMapWindow(display, window);
    XStoreName(display, window, title);
    wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);
    XSelectInput(display, window, KeyPressMask|KeyReleaseMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask);
    XVisualInfo visualInfo;
    if (!XMatchVisualInfo(display, screen, 24, DirectColor, &visualInfo)) {
        fprintf(stderr, "Error: Didn't match visual info!\n");
        return 1;
    }
    Visual* visual = visualInfo.visual;
    image.data = malloc(width * height * 4);
    image.width = width;
    image.height = height;
    image.depth = 4;
    image.size = width*height*4;
    image.stride = width*4;
    ximage = XCreateImage(display, visual, 24, ZPixmap, 0, (char*)image.data, width, height, 8, 0);
    return 0;
}

int checkWindowEvent_xlib(Event* event) {
    if (XPending(display) > 0) {
        XEvent xevent;
        XNextEvent(display, &xevent);
        switch(xevent.type) {
            case ClientMessage:
            if ((Atom)xevent.xclient.data.l[0] == wmDeleteMessage) {
                XDestroyImage(ximage);
                XCloseDisplay(display);
                event->type = WINDOW_CLOSE;
                return 1;
            }
            return 0;

            case KeyPress:
            event->type = KEY_CHANGE;
            event->keychange.state = 1;
            event->keychange.key = xevent.xkey.keycode - 8;
            return 1;

            case KeyRelease:
            event->type = KEY_CHANGE;
            event->keychange.state = 0;
            event->keychange.key = xevent.xkey.keycode - 8;
            return 1;

            case MotionNotify:
            event->type = MOUSE_MOVE;
            event->mousemove.x = xevent.xmotion.x;
            event->mousemove.y = xevent.xmotion.y;
            return 1;

            case ButtonPress:
            event->type = MOUSE_BUTTON;
            event->mousebutton.state = 1;
            event->mousebutton.button = xevent.xbutton.button;
            return 1;

            case ButtonRelease:
            event->type = MOUSE_BUTTON;
            event->mousebutton.state = 0;
            event->mousebutton.button = xevent.xbutton.button;
            return 1;

            default:
            return 0;
        }
    }
    return 0;
}

void updateWindow_xlib() {
    XPutImage(display, window, DefaultGC(display, screen), ximage, 0, 0, 0, 0, image.width, image.height);
    usleep(10000);
}