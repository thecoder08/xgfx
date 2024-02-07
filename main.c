#include <stdio.h>

const char service_interp[] __attribute__((section(".interp"))) = "/lib64/ld-linux-x86-64.so.2";

int main() {
  printf("This is the X11 Graphics Library (xgfx) version 2.0.\n");
  printf("Copyright (C) Lennon McLean 2023.\n");
  return 0;
}
