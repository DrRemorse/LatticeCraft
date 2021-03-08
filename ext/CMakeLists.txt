### NETWORK
set(TACO_SOURCES
  tacopie/sources/network/tcp_server.cpp
  tacopie/sources/network/unix/unix_tcp_socket.cpp
  tacopie/sources/network/unix/unix_self_pipe.cpp
  tacopie/sources/network/common/tcp_socket.cpp
  tacopie/sources/network/tcp_client.cpp
  tacopie/sources/network/io_service.cpp
  #tacopie/sources/network/windows/windows_self_pipe.cpp
  #tacopie/sources/network/windows/windows_tcp_socket.cpp
  tacopie/sources/utils/error.cpp
  tacopie/sources/utils/logger.cpp
  tacopie/sources/utils/thread_pool.cpp
)

add_library(tacopie ${TACO_SOURCES})
target_include_directories(tacopie PUBLIC tacopie/includes)

### GUI
option(NANOGUI_USE_GLAD "" OFF)
option(NANOGUI_BUILD_SHARED "" OFF)

set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)

add_subdirectory(nanogui)
# from manual:
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")
