#include <stdio.h>
#include <stdlib.h>
#include "window.h"
#include "window-wl.h"
#include "window-xcb.h"
#include "window-xlib.h"

int backend;

Image image;

void initWindow(int width, int height, const char* title) {
    printf("Trying Wayland backend...\n");
    if (!initWindow_wl(width, height, title)) {
        backend = 0;
        return;
    }
    printf("Trying XCB backend...\n");
    if (!initWindow_xcb(width, height, title)) {
        backend = 1;
        return;
    }
    printf("Trying Xlib backend...\n");
    if (!initWindow_xlib(width, height, title)) {
        backend = 2;
        return;
    }
    fprintf(stderr, "All backends failed, exiting\n");
    exit(1);
}

void updateWindow() {
    switch (backend) {
        case 0:
        updateWindow_wl();
        break;
        
        case 1:
        updateWindow_xcb();
        break;

        case 2:
        updateWindow_xlib();
        break;
    }
}

int checkWindowEvent(Event* event) {
    switch (backend) {
        case 0:
        return checkWindowEvent_wl(event);
        
        case 1:
        return checkWindowEvent_xcb(event);
        
        case 2:
        return checkWindowEvent_xlib(event);
    }
}