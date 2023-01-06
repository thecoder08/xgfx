#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "window.h"

Display* display;
Window window;
int screen;
XImage* image;
int width;
int height;

void initWindow(int localWidth, int localHeight, const char* title) {
    width = localWidth;
    height = localHeight;
    display = XOpenDisplay(NULL);
    screen = DefaultScreen(display);
    Window rootwindow = RootWindow(display, screen);
    window = XCreateSimpleWindow(display, rootwindow, 0, 0, width, height, 0, BlackPixel(display, screen), WhitePixel(display, screen));
    XMapWindow(display, window);
    XStoreName(display, window, title);
    XVisualInfo visualInfo;
    if (!XMatchVisualInfo(display, screen, 24, DirectColor, &visualInfo)) {
        printf("Error: Didn't match visual info!\n");
        return;
    }
    Visual* visual = visualInfo.visual;
    char* framebuffer = malloc(width * height * 6) + (width * height);
    image = XCreateImage(display, visual, 24, ZPixmap, 0, framebuffer, width, height, 8, 0);
}

void updateWindow() {
    XPutImage(display, window, DefaultGC(display, screen), image, 0, 0, 0, 0, width, height);
    usleep(10000);
}