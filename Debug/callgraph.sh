#!/bin/bash
set -e
pushd ../build
cmake .. -DGPROF=ON
make -j8
popd
../build/cppcraft
gprof ../build/cppcraft | gprof2dot | dot -Tpng -o gprof_callgraph.png
