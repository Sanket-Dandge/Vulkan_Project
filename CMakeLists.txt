cmake_minimum_required(VERSION 3.18)
project(Uriel LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(X11 REQUIRED)

include_directories(
    include
    src
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_subdirectory(src)
