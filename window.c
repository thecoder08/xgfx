#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "window.h"

#define new_min(x,y) (((x) <= (y)) ? (x) : (y))

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
    int eventsToRead = new_min(XPending(display), eventBufferSize);
    for (int i = 0; i < eventsToRead; i++) {
        XEvent* event = &eventBuffer[i];
        XNextEvent(display, event);
        if ((event->type == ClientMessage) && ((Atom)event->xclient.data.l[0] == wmDeleteMessage)) {
            XDestroyImage(image);
            XCloseDisplay(display);
            event->type = ClosedWindow;
            return 1;
        }
    }
    return eventsToRead;
}

void updateWindow() {
    XPutImage(display, window, DefaultGC(display, screen), image, 0, 0, 0, 0, image->width, image->height);
    usleep(10000);
}