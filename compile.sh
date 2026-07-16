#!/bin/sh

#skybox
glslc ./assets/shaders/texturecubemap/skybox.vert -o ./assets/spv/skybox.vert.spv
glslc ./assets/shaders/texturecubemap/skybox.frag -o ./assets/spv/skybox.frag.spv

glslc ./assets/shaders/texturecubemap/reflect.vert -o ./assets/spv/reflect.vert.spv
glslc ./assets/shaders/texturecubemap/reflect.frag -o ./assets/spv/reflect.frag.spv

# Tessellation
glslc ./assets/shaders/tessellation/base.vert -o ./assets/spv/base.vert.spv
glslc ./assets/shaders/tessellation/base.frag -o ./assets/spv/base.frag.spv

glslc ./assets/shaders/tessellation/ts_filled.tesc -o ./assets/spv/ts_filled.tesc.spv
glslc ./assets/shaders/tessellation/ts_wire.tese -o ./assets/spv/ts_wire.tese.spv

glslc ./assets/shaders/tessellation/non_filled.tesc -o ./assets/spv/non_filled.tesc.spv
glslc ./assets/shaders/tessellation/non_wire.tese -o ./assets/spv/non_wire.tese.spv

# Triangle
glslc ./assets/shaders/triangle/triangle.vert -o ./assets/spv/triangle.vert.spv
glslc ./assets/shaders/triangle/triangle.frag -o ./assets/spv/triangle.frag.spv
