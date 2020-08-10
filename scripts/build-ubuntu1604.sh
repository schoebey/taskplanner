#!/usr/bin/env bash

mkdir build
cd build

export PATH="/opt/cmake-3.18.1-Linux-x86_64/bin:/opt/ninja-1.10:$PATH"

QT_ENV_SCRIPT=$(find /opt -name 'qt*-env.sh')
source $QT_ENV_SCRIPT

cmake -G"Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/usr ..

cmake --build . --target all -- -j$(nproc)

DESTDIR=install cmake --build . --target install

wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
export LD_LIBRARY_PATH=$(pwd)/install/usr/lib
export VERSION="v0.8.0"
./linuxdeployqt-continuous-x86_64.AppImage install/usr/share/applications/*.desktop -appimage
