### GUI
# option(NANOGUI_USE_GLAD "" OFF)
# option(NANOGUI_BUILD_SHARED "" OFF)

set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)

include_directories(BEFORE nanogui/include)
include_directories(BEFORE nanogui/ext/eigen)
include_directories(BEFORE nanogui/ext/nanovg/src)
include_directories(BEFORE nanogui/ext/glfw/include)

add_subdirectory(nanogui)
# from manual:
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")
