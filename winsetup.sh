#!/bin/bash
mkdir -p build
pushd build
rm -rf *
cmake -G "MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND" ..
mingw32-make -j
popd
