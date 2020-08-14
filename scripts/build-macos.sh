#!/usr/bin/env bash

mkdir build
cd build

cmake -G"Unix Makefiles" -DCMAKE_INSTALL_PREFIX=. ..

cmake --build . --target all -- -j

cmake --build . --target install

/usr/local/Cellar/qt/5.15.0/bin/macdeployqt taskplanner.app

mkdir taskplanner
mv taskplanner.app taskplanner/
cd taskplanner
ln -s /Applications .
cd ..

hdiutil create -srcfolder taskplanner taskplanner.dmg

mv taskplanner.dmg ../
