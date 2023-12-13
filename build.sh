#!/bin/sh
VERSION=1.8.2
gcc -shared window.c drawing.c main.c -fPIC /usr/lib/x86_64-linux-gnu/Scrt1.o -lX11 -o libxgfx.so
gcc -shared window-wl.c drawing.c main.c xdg-shell-protocol.c -fPIC /usr/lib/x86_64-linux-gnu/Scrt1.o -lwayland-client -o libxgfx-wl.so
mkdir -p libxgfx/lib/x86_64-linux-gnu libxgfx/include/xgfx
mv libxgfx.so libxgfx-wl.so libxgfx/lib/x86_64-linux-gnu
cp drawing.h window.h window-wl.h libxgfx/include/xgfx
tar -czvf libxgfx_${VERSION}_1_amd64.tar.gz libxgfx
mkdir -p libxgfx_${VERSION}_1_amd64/DEBIAN
cp control libxgfx_${VERSION}_1_amd64/DEBIAN
mv libxgfx libxgfx_${VERSION}_1_amd64/usr
dpkg-deb --build libxgfx_${VERSION}_1_amd64
rm -r libxgfx_${VERSION}_1_amd64
