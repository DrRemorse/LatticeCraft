#!/bin/bash
sudo apt install libluajit-5.1-dev liblua5.1-0-dev
set -e
git submodule update --init --recursive
mkdir -p build
pushd build
cmake ..
make -j
popd
