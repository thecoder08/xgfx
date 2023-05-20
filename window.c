#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "window.h"

Display* display;
Window window;
int screen;
XImage* image;
Atom wmDeleteMessage;

void initWindow(int width, int height, const char* title) {
    display = XOpenDisplay(NULL);
    screen = DefaultScreen(display);
    window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, width, height, 0, BlackPixel(display, screen), WhitePixel(display, screen));
    XMapWindow(display, window);
    XStoreName(display, window, title);
    wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);
    XSelectInput(display, window, KeyPressMask|KeyReleaseMask);
    XkbSetDetectableAutoRepeat(display, 1, NULL);
    XVisualInfo visualInfo;
    if (!XMatchVisualInfo(display, screen, 24, DirectColor, &visualInfo)) {
        printf("Error: Didn't match visual info!\n");
        return;
    }
    Visual* visual = visualInfo.visual;
    char* framebuffer = malloc(width * height * 4);
    image = XCreateImage(display, visual, 24, ZPixmap, 0, framebuffer, width, height, 8, 0);
}

int checkWindowEvents(XEvent* eventBuffer, int eventBufferSize) {
    int count = 0;
    while ((XPending(display) > 0) && (count != eventBufferSize)) {
        XEvent* event = eventBuffer + (sizeof(XEvent) * count);
        XNextEvent(display, event);
        if ((event->type == ClientMessage) && ((Atom)event->xclient.data.l[0] == wmDeleteMessage)) {
            XDestroyImage(image);
            XCloseDisplay(display);
            // we set the very first event
            eventBuffer->type = ClosedWindow;
            return 1;
        }
        count++;
    }
    return count;
}

void updateWindow() {
    XPutImage(display, window, DefaultGC(display, screen), image, 0, 0, 0, 0, image->width, image->height);
    usleep(10000);
}