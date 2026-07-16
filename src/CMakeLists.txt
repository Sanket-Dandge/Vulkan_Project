file(GLOB_RECURSE ENGINE_SOURCES
    "*.cpp"
)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

add_library(Uriel STATIC ${ENGINE_SOURCES})

target_include_directories(Uriel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Uriel PUBLIC
    glfw
    Vulkan::Vulkan
    X11::X11
    imgui
    ktx
)

target_compile_definitions(Uriel PRIVATE
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:RelWithDebInfo>:NDEBUG>
    $<$<CONFIG:MinSizeRel>:NDEBUG>
)
