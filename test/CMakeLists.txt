cmake_minimum_required (VERSION 3.0.2)
project (cppcraft C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
add_definitions(-DCPPCRAFT_CLIENT)

include_directories(../common)
include_directories(../src)
include_directories(../library/include)
include_directories(../inc)
include_directories(../inc/glm)
include_directories(Catch/include)

set(SOURCES
    test_gridwalker.cpp
    test_lighting.cpp
    test_readonly_blocks.cpp
    test_sector.cpp
    catch.cpp
    mock_generator.cpp
    mock_player.cpp
    mock_precompq.cpp
    mock_sectors.cpp
    mock_stuff.cpp
    #../src/db/blockdata.cpp
    ../src/gameconf.cpp
    ../src/lighting.cpp
    ../src/lighting_algos.cpp
    ../src/lighting_remove.cpp
    ../src/light_correction.cpp
    ../src/sector.cpp
    ../src/sectors.cpp
    ../src/spiders.cpp
    ../src/spiders_modify.cpp
    ../src/spiders_world.cpp
    ../src/world.cpp
    ../common/readonly_blocks.cpp
  )

set(LIB_SOURCES
    ../library/library/config.cpp
    ../library/library/log.cpp
    ../library/library/bitmap/bitmap.cpp
    ../library/library/bitmap/lodepng.cpp
    ../library/library/opengl/fbo.cpp
    ../library/library/opengl/opengl.cpp
    ../library/library/opengl/texture.cpp
  )

include(FindPkgConfig)
pkg_search_module(GLFW REQUIRED glfw3)

add_executable(unittests ${SOURCES} ${LIB_SOURCES})
target_link_libraries(unittests ${GLFW_LIBRARIES} libGLEW.a GL)
