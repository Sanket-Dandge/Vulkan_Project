#include "core/shaders.h"
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace capella {
    Shader::Shader(VkDevice& ctxDevice) {
        device = ctxDevice;
    }

    Shader::~Shader() {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }

    /*
     *  @info: compiles the GLSL code into SPV code and stores in a file
     *  Also, checks if there is any need for recompilation using last code change timestamp
     * */
    void Shader::compileSPVFile(const std::string& glslPath, bool isVertexShader) {
        std::string spv_path;
        bool needCompile = true;

        if (std::filesystem::exists(spv_path)) {
            auto spv_time = std::filesystem::last_write_time(spv_path);
            auto glsl_time = std::filesystem::last_write_time(glslPath);
            needCompile = glsl_time > spv_time;
        }

        if (needCompile) {
            std::ifstream file(glslPath);
            if (!file) {
                LOG_ERROR(TAG, "failed to open GLSL file: %s", glslPath.c_str());
                std::terminate();
            }

            std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            auto spirv = compileGLSLtoSpv(source, glslPath, isVertexShader);

            std::ofstream outputFile(spv_path, std::ios::binary);
            outputFile.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
        }
    }

    //  @info: code for compiling GLSL code to SPV code
    std::vector<uint32_t> Shader::compileGLSLtoSpv(std::string source, std::string filePath, bool isVertexShader) {
        shaderc::Compiler shader_compiler;
        shaderc::CompileOptions shader_options;
        //  TODO: Change the version according to VkApplicationInfo given
        shader_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        //  TODO: Check whether spirv version should be dependent on vulkan versin or not
        shader_options.SetTargetSpirv(shaderc_spirv_version_1_5);

        shaderc_shader_kind shader_kind = isVertexShader ? shaderc_glsl_vertex_shader : shaderc_glsl_fragment_shader;
        shaderc::SpvCompilationResult result = shader_compiler.CompileGlslToSpv(source, shader_kind, filePath.c_str());

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            throw std::runtime_error(result.GetErrorMessage());
        }

        return {result.cbegin(), result.cend()};
    }


    // @info: function for reading the spv file
    std::vector<char> Shader::read(std::string filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            LOG_ERROR(TAG, "failed to open the file: %s", filename.c_str());
            std::terminate();
        }

        size_t file_size = (size_t)file.tellg();
        std::vector<char> fileData(file_size);

        file.seekg(0);
        file.read(fileData.data(), file_size);
        file.close();

        return fileData;
    }

    //  @info: shadr module creation
    VkShaderModule Shader::createModule(const std::vector<char>& spvCode) {
        VkShaderModuleCreateInfo moduleCreateInfo{};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = spvCode.size();
        moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(spvCode.data());

        VkShaderModule shader_module;
        if (vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shader_module) != VK_SUCCESS) {
            LOG_ERROR(TAG, "failed to create shader module");
            std::terminate();
        }

        return shader_module;
    }

    //  @info: loads the shader
    void Shader::load(std::string path, bool isVertexShader) {
        std::string spvPath = path + ".spv";

        if (std::filesystem::exists(path)) {
            compileSPVFile(path, isVertexShader);
        } else {
            LOG_ERROR(TAG, "path does not exist: %s", path.c_str());
            std::terminate();
        }

        auto code = read(spvPath);
        shaderModule = createModule(code);
    }

}
