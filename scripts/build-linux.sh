#!/usr/bin/env bash

mkdir build
cd build
cmake -GNinja -DCMAKE_INSTALL_PREFIX=/usr ..
cmake --build . --target all -- -j
DESTDIR=install cmake --build . --target install

#TODO: linuxdeployqt
