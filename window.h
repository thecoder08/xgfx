void initWindow(int width, int height, const char* title);
void updateWindow();
#include <X11/Xlib.h>
int checkWindowEvents(XEvent* event);