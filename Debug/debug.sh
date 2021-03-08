#!/bin/bash
set -e
pushd ../build
cmake .. -DGPROF=OFF
if [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    mingw32-make -j8
else
    make -j8
fi
popd
../build/cppcraft
