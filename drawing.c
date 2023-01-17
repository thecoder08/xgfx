#include <X11/Xutil.h>
#include <stdlib.h>
#include "drawing.h"

extern XImage* image;
extern int width;
extern int height;

void plot(int x, int y, int color) {
  if (x > 0 && x < width && y > 0 && y < height) {
    XPutPixel(image, x, y, color);
  }
}

void rectangle(int x, int y, int width, int height, int color) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      plot(x + j, y + i, color);
    }
  }
}

void circle(int x, int y, int radius, int color) {
  for (int i = -radius; i < radius; i++) {
    for (int j = -radius; j < radius; j++) {
      if (j*j + i*i < radius*radius) {
        plot(x + j, y + i, color);
      }
    }
  }
}

void line(int x0, int y0, int x1, int y1, int color) {
   int dx = abs(x1 - x0);
   int dy = abs(y1 - y0);
   int sx = (x0 < x1) ? 1 : -1;
   int sy = (y0 < y1) ? 1 : -1;
   int err = dx - dy;
   while(1) {
      plot(x0, y0, color);
      if ((x0 == x1) && (y0 == y1)) break;
      int e2 = 2*err;
      if (e2 > -dy) { err -= dy; x0  += sx; }
      if (e2 < dx) { err += dx; y0  += sy; }
   }
}