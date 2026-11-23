file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "*.cpp"
)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp"
)

target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)

target_link_libraries(imgui PUBLIC
    glfw
    Vulkan::Vulkan
)

# Remove the test entry point from the engine library sources.
list(REMOVE_ITEM ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

add_library(Capella STATIC ${ENGINE_SOURCES})

target_include_directories(Capella PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Capella PUBLIC
    glfw
    Vulkan::Vulkan
    X11::X11
    imgui
    ktx
)

target_compile_options(Capella PRIVATE
    "-fmacro-prefix-map=${CMAKE_SOURCE_DIR}/="
)

add_executable(test main.cpp)
target_link_libraries(test PRIVATE Capella)
