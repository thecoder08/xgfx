# The X11 Graphics Library
The X11 Graphics Library (libxgfx) is a simple graphics library that allows you to create simple games like the pong example shown [here](pong-demo.c).
## Installing
You can install libxgfx on Debian by downloading the .deb file from the releases page, or by using the tarball which includes the library and headers.
## Using
libxgfx includes two header files, window.h, for creating and updating a window with graphics, and drawing.h, for drawing to a framebuffer that will be displayed on the window. Check out [pong-demo.c](pong-demo.c) for an overview of how to use the window functions and drawing primitives. The code is self-documenting.

You can compile your code with the library by adding `-lxgfx` to your compiler options.