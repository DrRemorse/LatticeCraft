#!/bin/bash
INC="-Isrc -Iinc -Iinc/glm -Iinc/LuaBridge/Source/LuaBridge -Iinc/rapidjson/include -Iinc/SimplexNoise/include -Ilibrary/include -Iext/tacopie/includes -Iext/nanogui/include -Iext/nanogui/ext/eigen -Iext/nanogui/ext/nanovg/src"
DEF="-DGLM_ENABLE_EXPERIMENTAL"
CHK="clang-analyzer-*,bugprone-*,modernize-*,performance-*,misc-*"

clang-tidy-5.0 -checks=$CHK src/precomp*.cpp -- -std=c++14 $INC $DEF
