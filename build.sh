#!/bin/sh
VERSION=2.4
gcc -shared main.c window.c window-wl.c window-xcb.c window-xlib.c drawing.c xdg-shell.c -fPIC /usr/lib/x86_64-linux-gnu/Scrt1.o -o libxgfx.so -lwayland-client -lX11 -lxcb -lxcb-shm
strip --strip-all libxgfx.so
mkdir -p libxgfx/lib/x86_64-linux-gnu libxgfx/include/xgfx
mv libxgfx.so libxgfx/lib/x86_64-linux-gnu
cp drawing.h window.h libxgfx/include/xgfx
tar -czvf libxgfx_${VERSION}_1_amd64.tar.gz libxgfx
mkdir -p libxgfx_${VERSION}_1_amd64/DEBIAN
cp control libxgfx_${VERSION}_1_amd64/DEBIAN
mv libxgfx libxgfx_${VERSION}_1_amd64/usr
dpkg-deb --build libxgfx_${VERSION}_1_amd64
rm -r libxgfx_${VERSION}_1_amd64