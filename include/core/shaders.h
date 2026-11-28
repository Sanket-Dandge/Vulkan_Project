#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "helpers/logger.h"

namespace capella {
    class Shader {
        public:
            Shader() {}
            Shader(VkDevice& device);
            void load(std::string path, bool isVertexShader);
            std::vector<uint32_t> compileGLSLtoSpv(std::string source, std::string filePath, bool isVertexShader);
            std::vector<char> read(std::string filename);

            void compileSPVFile(const std::string& glsl_path, bool isVertexShader);
            VkShaderModule createModule(const std::vector<char>& spvCode);

            ~Shader();

        private:
            VkDevice device;
            VkShaderModule shaderModule;
            const char* TAG = "Shader";

    };
}


#endif
