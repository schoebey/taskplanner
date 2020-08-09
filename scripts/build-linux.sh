#!/usr/bin/env bash

mkdir build
cd build

QT_ENV_SCRIPT=$(find /opt -name 'qt*-env.sh')
source $QT_ENV_SCRIPT

cmake -G"Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/usr ..

cmake --build . --target all -- -j

DESTDIR=install cmake --build . --target install

#TODO: linuxdeployqt
