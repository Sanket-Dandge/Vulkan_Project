#include "core/pipelines.h"
#include <vulkan/vulkan_core.h>

namespace capella {

    void Pipeline::createPipelineCache() {
        VkPipelineCacheCreateInfo pipelineCacheInfo{};
        pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        if (!vkCreatePipelineCache(device, &pipelineCacheInfo, nullptr, &mPipelineCache)) {
            LOG_ERROR(TAG, "Failed to create pipeline cache");
        }
    }

    //  @info: creates the graphics pipeline for the rendering
    void Pipeline::createPipelineLayout(PipelineConfig& info) {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = pipelineInfo.pushConstants.size();
        pipelineLayoutInfo.pPushConstantRanges = pipelineInfo.pushConstants.data();
        if(!vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &mPipelineLayout)) {
            LOG_ERROR(TAG, "Failed to create pipeline layout");
            std::terminate();
        }

        LOG_INFO(TAG, "Successfully created the pipeline layout.");
    }

    void Pipeline::createGraphicsPipeline() {
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

        int count = 0;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStagesArray;
        if (pipelineInfo.vertexShader != VK_NULL_HANDLE) {
            shaderStagesArray.push_back(addVertexShaderInfo(pipelineInfo.vertexShader));
            count++;
        }

        if (pipelineInfo.fragmentShader != VK_NULL_HANDLE) {
            shaderStagesArray.push_back(addFragmentShaderInfo(pipelineInfo.fragmentShader));
            count++;
        }

        if (pipelineInfo.tessellationControlShader != VK_NULL_HANDLE) {
            shaderStagesArray.push_back(addTessellationShaderInfo(pipelineInfo.tessellationControlShader));
            count++;
        }
        
        pipelineCreateInfo.stageCount = count;
        pipelineCreateInfo.pStages = shaderStagesArray.data();

        auto vertexInputInfo = addVertexInputInfo();
        auto inputAssemblyInfo = addInputAssemblyInfo();
        auto viewportInfo = addViewportInfo();
        auto rasterizationInfo = addRasterizationInfo();
        auto multisampleInfo = addMultisampleInfo();
        auto colorBlendStateInfo = addColorBlendStateInfo(addColorBlendAttachmentInfo());
        auto depthStencilInfo = addDepthStencilInfo();
        auto dynamicStateInfo = addDynamicStateInfo();

        pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
        pipelineCreateInfo.pViewportState = &viewportInfo;
        pipelineCreateInfo.pRasterizationState = &rasterizationInfo;
        pipelineCreateInfo.pMultisampleState = &multisampleInfo;
        pipelineCreateInfo.pColorBlendState = &colorBlendStateInfo;
        pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
        pipelineCreateInfo.pDynamicState = &dynamicStateInfo;
        pipelineCreateInfo.layout = mPipelineLayout;
        pipelineCreateInfo.renderPass = renderpass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, mPipelineCache, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS) {
            LOG_ERROR(TAG, "Failed to create graphics pipeilne");
            std::terminate();
        }

        LOG_INFO(TAG, "Successfully created the graphics pipeline.");
    }

    //  @info: defines vertex shader stage info for pipeline creation
    VkPipelineShaderStageCreateInfo Pipeline::addVertexShaderInfo(VkShaderModule& vertexModule) {
        pipelineInfo.vertexShader = vertexModule;
        VkPipelineShaderStageCreateInfo vertexCreateInfo{};
        vertexCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexCreateInfo.module = pipelineInfo.vertexShader;
        vertexCreateInfo.pName = "main";

        return vertexCreateInfo;
    }

    //  @info: defines fragment shader stage info for pipeline creation
    VkPipelineShaderStageCreateInfo Pipeline::addFragmentShaderInfo(VkShaderModule& fragmentModule) {
        pipelineInfo.fragmentShader = fragmentModule;
        VkPipelineShaderStageCreateInfo fragmentCreateInfo{};
        fragmentCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        fragmentCreateInfo.module = pipelineInfo.fragmentShader;
        fragmentCreateInfo.pName = "main";

        return fragmentCreateInfo;
    }

    //  @info: defines tessellation control shader stage info for pipeline creation
    VkPipelineShaderStageCreateInfo Pipeline::addTessellationShaderInfo(VkShaderModule& tessellationModule) {
        pipelineInfo.tessellationControlShader = tessellationModule;
        VkPipelineShaderStageCreateInfo tessellationCreateInfo{};
        tessellationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        tessellationCreateInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        tessellationCreateInfo.module = pipelineInfo.tessellationControlShader;
        tessellationCreateInfo.pName = "main";
        
        return tessellationCreateInfo;
    }

    //  @info: creates the vertex input state info defining the binding and attribute description of data
    const VkPipelineVertexInputStateCreateInfo Pipeline::addVertexInputInfo() {
        auto bindingDescription = Vertex::getBindingDescription(pipelineInfo.vertexInputBinding);
        auto attributeDescription = Vertex::getAttributeDescription(pipelineInfo.vertexInputBinding, pipelineInfo.vertexComponents);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

        return vertexInputInfo;
    }

    //  @info: defines the primitive topology of the object
    VkPipelineInputAssemblyStateCreateInfo Pipeline::addInputAssemblyInfo() {
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = pipelineInfo.topology;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        return inputAssemblyInfo;
    }

    //  @info: defines the transformation of image to framebuffer and the regions where pixels will be stored
    VkPipelineViewportStateCreateInfo Pipeline::addViewportInfo() {
        VkPipelineViewportStateCreateInfo viewportInfo{};
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

        if (pipelineInfo.viewportCount > 1) {
            viewportInfo.viewportCount = pipelineInfo.viewportCount;
            viewportInfo.pViewports = pipelineInfo.viewports.data();
        } else if (pipelineInfo.viewportCount == 1) {
            viewportInfo.viewportCount = pipelineInfo.viewportCount;
        }

        if (pipelineInfo.scissorCount > 1) {
            viewportInfo.scissorCount = pipelineInfo.scissorCount;
            viewportInfo.pScissors = pipelineInfo.scissors.data();
        } else if (pipelineInfo.scissorCount == 1) {
            viewportInfo.scissorCount = pipelineInfo.scissorCount;
        }

        return viewportInfo;
    }

    //  @info: defines how geometry be convertex into fragments (i.e 3D to 2D conversion)
    VkPipelineRasterizationStateCreateInfo Pipeline::addRasterizationInfo() {
        VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
        rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerInfo.depthClampEnable = pipelineInfo.depthClamp;
        rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizerInfo.polygonMode = pipelineInfo.polygonMode;
        rasterizerInfo.lineWidth = 1.f;
        rasterizerInfo.cullMode = pipelineInfo.cullMode;
        rasterizerInfo.frontFace = pipelineInfo.frontFace;
        rasterizerInfo.depthBiasEnable = pipelineInfo.depthBias;

        return rasterizerInfo;
    }

    VkPipelineMultisampleStateCreateInfo Pipeline::addMultisampleInfo() {
        VkPipelineMultisampleStateCreateInfo multisampleInfo{};
        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.sampleShadingEnable = VK_FALSE;
        multisampleInfo.rasterizationSamples = pipelineInfo.msaaSamples;

        return multisampleInfo;
    }

    VkPipelineDepthStencilStateCreateInfo Pipeline::addDepthStencilInfo() {
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilInfo.depthTestEnable = pipelineInfo.enableDepthTest;
        depthStencilInfo.depthWriteEnable = pipelineInfo.enableDepthWrite;
        depthStencilInfo.depthCompareOp = pipelineInfo.depthOperator;
        depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilInfo.stencilTestEnable = pipelineInfo.enableStencilTest;

        return depthStencilInfo;
    }

    VkPipelineColorBlendAttachmentState Pipeline::addColorBlendAttachmentInfo() {
        VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo;
        colorBlendAttachmentInfo.colorWriteMask = pipelineInfo.colorComponentFlags;
        colorBlendAttachmentInfo.blendEnable = pipelineInfo.enableBlend;
        colorBlendAttachmentInfo.srcColorBlendFactor = pipelineInfo.srcColorBlendFactor;
        colorBlendAttachmentInfo.dstColorBlendFactor = pipelineInfo.dstColorBlendFactor;
        colorBlendAttachmentInfo.colorBlendOp = pipelineInfo.colorBlendOperator;

        colorBlendAttachmentInfo.srcAlphaBlendFactor = pipelineInfo.srcAlphaBlendFactor;
        colorBlendAttachmentInfo.dstAlphaBlendFactor = pipelineInfo.dstAlphaBlendFactor;
        colorBlendAttachmentInfo.alphaBlendOp = pipelineInfo.alphaBlendOpeator;

        return colorBlendAttachmentInfo;
    }

    VkPipelineColorBlendStateCreateInfo Pipeline::addColorBlendStateInfo(VkPipelineColorBlendAttachmentState attachment) {
        VkPipelineColorBlendStateCreateInfo colorBlendStateInfo{};
        colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateInfo.attachmentCount = 1;
        colorBlendStateInfo.pAttachments = &attachment;

        return colorBlendStateInfo;
    }

    VkPipelineDynamicStateCreateInfo Pipeline::addDynamicStateInfo() {
        VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.pDynamicStates = pipelineInfo.dynamicStateEnables.data();
        dynamicStateInfo.dynamicStateCount = pipelineInfo.dynamicStateEnables.size();

        return dynamicStateInfo;
    }

    VkPushConstantRange Pipeline::addPushConstant() {
        VkPushConstantRange pushConstantRangeInfo{};
        pushConstantRangeInfo.stageFlags = pipelineInfo.pushConstantStageFlags;
        pushConstantRangeInfo.offset = 0;
        pushConstantRangeInfo.size = pipelineInfo.pushConstantSize;

        return pushConstantRangeInfo;
    }

    void Pipeline::destroy() {
        if (mPipelineCache != VK_NULL_HANDLE) {
            vkDestroyPipelineCache(device, mPipelineCache, nullptr);
        }

        vkDestroyPipeline(device, mPipeline, nullptr);
        vkDestroyPipelineLayout(device, mPipelineLayout, nullptr);
    }

}
