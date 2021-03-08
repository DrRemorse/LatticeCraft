cmake_minimum_required (VERSION 3.0.2)
project(server C CXX)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g -flto=thin")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread -stdlib=libc++ -flto=thin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=thread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(BUILD_SHARED_LIBRARIES OFF)

option(GPROF "Enable gprof profiling" OFF)
if (GPROF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif()
option(GPERF "Enable gperf profiling" OFF)
if (GPERF)
add_definitions(-DENABLE_GPERF)
endif()

include(FindPkgConfig)
#pkg_check_modules(LUA REQUIRED lua-5.1)
#include_directories("${LUA_STATIC_INCLUDE_DIRS}")
#link_directories("${LUA_STATIC_LIBRARY_DIRS}")
set(LUA_LIBS luajit-5.1)

add_definitions(-DCPPCRAFT_SERVER)
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

include_directories(common)
include_directories(library/include)
include_directories(ext/cbuffer/include)
include_directories(../inc)
include_directories(../inc/glm)
include_directories(../inc/LuaBridge/Source/LuaBridge)
include_directories(../inc/rapidjson/include)
include_directories(../inc/SimplexNoise/include)

add_subdirectory(common)
add_subdirectory(library)
add_subdirectory(src)
add_subdirectory(ext/cbuffer)

### linker stage ###
link_directories(${CMAKE_BINARY_DIR})
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld-5.0")

add_executable(server ${SOURCES})
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(server -pthread)
target_link_libraries(server library common cbuffer ${LUA_LIBS})

if (GPERF)
  target_link_libraries(server /usr/local/lib/libprofiler.a)
endif()

install(TARGETS server DESTINATION ${CMAKE_INSTALL_PREFIX})
