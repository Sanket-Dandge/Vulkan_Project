#ifndef PIPELINE_H
#define PIPELINE_H

#include <exception>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "helpers/logger.h"

//  @info; different components of vertex
enum class VertexComponent {
    Position,
    TexCoord,
    Normal,
    Color,
    Joint0,
    Weight0,
    Tangent
};

//  @info: defines the parameters for a pipeline
struct PipelineConfig {
    //  #info: shaderstagecreateinfo params
    VkShaderModule vertexShader = VK_NULL_HANDLE;
    VkShaderModule fragmentShader = VK_NULL_HANDLE;
    VkShaderModule tessellationControlShader = VK_NULL_HANDLE;

    //  #info: vertexInputInfo params
    std::vector<VertexComponent>& vertexComponents;
    uint32_t vertexInputBinding;

    //  @info: inputAssemblyInfo params
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    //  @info: viewportInfo params
    uint32_t viewportCount;
    uint32_t scissorCount;
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;

    //  @info: rasterizationInfo params
    VkBool32 depthClamp = VK_FALSE;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkBool32 depthBias = VK_FALSE;
    VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

    //  @info: multisamplingInfo params
    VkSampleCountFlagBits msaaSamples;

    //  @info: depthStencilInfo params
    VkBool32 enableDepthTest = VK_TRUE;
    VkBool32 enableDepthWrite = VK_TRUE;
    VkBool32 enableStencilTest = VK_FALSE;
    VkCompareOp depthOperator = VK_COMPARE_OP_LESS;

    //  @info: colorBlendAttachmentInfo params
    VkBool32 enableBlend = VK_TRUE;
    VkColorComponentFlags colorComponentFlags = VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
    VkBlendFactor srcColorBlendFactor;
    VkBlendFactor dstColorBlendFactor;
    VkBlendOp colorBlendOperator;

    VkBlendFactor srcAlphaBlendFactor;
    VkBlendFactor dstAlphaBlendFactor;
    VkBlendOp alphaBlendOpeator;

    //  @info: dynamicStateInfo params
    std::vector<VkDynamicState> dynamicStateEnables;

    std::vector<VkPushConstantRange> pushConstants;
    VkShaderStageFlags pushConstantStageFlags;

    //  @note: the size should be of pushconstant struct
    size_t pushConstantSize;

    
};

//  @info: Vertex struct defining the attributes, binding and attribute descriptions of vertex components
struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec4 color;
    glm::uvec4 joint0;
    glm::vec4 weight0;
    glm::vec4 tangent;

    //  @info: description for binding vertex components
    static VkVertexInputBindingDescription getBindingDescription(uint32_t binding) {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = binding;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    //  @info: defines the attribute description of a single vertex component
    static VkVertexInputAttributeDescription getAttrDescOfVertexComponent(uint32_t binding, uint32_t location, VertexComponent component) {
        switch (component) {
            case VertexComponent::Position: 
                return VkVertexInputAttributeDescription({ .location=location, .binding=binding, .format=VK_FORMAT_R32G32B32_SFLOAT, .offset=offsetof(Vertex, position) });
            case VertexComponent::Normal:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
            case VertexComponent::TexCoord:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord) });
            case VertexComponent::Color:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color) });
            case VertexComponent::Tangent:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, tangent)} );
            case VertexComponent::Joint0:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, joint0) });
            case VertexComponent::Weight0:
                return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, weight0) });
            default:
                return VkVertexInputAttributeDescription({});
        }
    }

    //  @info: defines attribute description of all required vertex components
    //  @impNote: order of the components are important as location would be done in serial order of the elements present in the compoonents vector
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescription(uint32_t binding, const std::vector<VertexComponent> components) {
        std::vector<VkVertexInputAttributeDescription> res;
        uint32_t location = 0;
        for (auto component : components) {
            res.push_back(getAttrDescOfVertexComponent(binding, location, component));
            location++;
        }
        return res;
    }

};

namespace capella {
    class Pipeline {
        public:
            PipelineConfig pipelineInfo;

            void createPipelineCache();
            void createPipelineLayout(PipelineConfig& info);
            void createGraphicsPipeline();

            VkPipelineShaderStageCreateInfo addVertexShaderInfo(VkShaderModule& vertexModule);
            VkPipelineShaderStageCreateInfo addFragmentShaderInfo(VkShaderModule& fragmentModule);
            VkPipelineShaderStageCreateInfo addTessellationShaderInfo(VkShaderModule& tessellationModule);

            const VkPipelineVertexInputStateCreateInfo addVertexInputInfo();
            VkPipelineInputAssemblyStateCreateInfo addInputAssemblyInfo();

            VkPipelineViewportStateCreateInfo addViewportInfo();

            VkPipelineRasterizationStateCreateInfo addRasterizationInfo();
            VkPipelineMultisampleStateCreateInfo addMultisampleInfo();

            VkPipelineDepthStencilStateCreateInfo addDepthStencilInfo();
            VkPipelineColorBlendAttachmentState addColorBlendAttachmentInfo();

            VkPipelineColorBlendStateCreateInfo addColorBlendStateInfo(VkPipelineColorBlendAttachmentState attachment);
            VkPipelineDynamicStateCreateInfo addDynamicStateInfo();

            VkPushConstantRange addPushConstant();

            void destroy();

        private:
            VkPipelineCache mPipelineCache;
            VkPipeline mPipeline;
            VkPipelineLayout mPipelineLayout;

            VkDescriptorSetLayout descriptorSetLayout;
            VkDevice device;
            VkRenderPass renderpass;

            const char* TAG = "Pipeline";

    };
}


#endif
