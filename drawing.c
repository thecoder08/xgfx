#include <X11/Xutil.h>
#include "drawing.h"

extern XImage* image;

void rectangle(int x, int y, int width, int height, int color) {
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      XPutPixel(image, x + j, y + i, color);
    }
  }
}

void circle(int x, int y, int radius, int color) {
  for (int i = -radius; i < radius; i++) {
    for (int j = -radius; j < radius; j++) {
      if (j*j + i*i < radius*radius) {
        XPutPixel(image, x + j, y + i, color);
      }
    }
  }
}