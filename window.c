#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "window.h"

#define new_min(x,y) (((x) <= (y)) ? (x) : (y))

struct Image {
  int width;
  int height;
  int depth;
  int stride;
  int size;
  int* data;
};

struct Image* image;

Display* display;
Window window;
int screen;
XImage* ximage;
Atom wmDeleteMessage;

void initWindow(int width, int height, const char* title) {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
      fprintf(stderr, "Failed to connect to display!\n");
      exit(1);
    }
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
        exit(1);
    }
    Visual* visual = visualInfo.visual;
    image = malloc(sizeof(struct Image));
    void* framebuffer = malloc(width * height * 4);
    image->data = framebuffer;
    image->width = width;
    image->height = height;
    image->depth = 4;
    image->size = width*height*4;
    image->stride = width*4;
    ximage = XCreateImage(display, visual, 24, ZPixmap, 0, framebuffer, width, height, 8, 0);
}

int checkWindowEvents(XEvent* eventBuffer, int eventBufferSize) {
    int eventsToRead = new_min(XPending(display), eventBufferSize);
    for (int i = 0; i < eventsToRead; i++) {
        XEvent* event = &eventBuffer[i];
        XNextEvent(display, event);
        if ((event->type == ClientMessage) && ((Atom)event->xclient.data.l[0] == wmDeleteMessage)) {
            XDestroyImage(ximage);
            XCloseDisplay(display);
            event->type = ClosedWindow;
            return 1;
        }
    }
    return eventsToRead;
}

void updateWindow() {
    XPutImage(display, window, DefaultGC(display, screen), ximage, 0, 0, 0, 0, image->width, image->height);
    usleep(10000);
}
