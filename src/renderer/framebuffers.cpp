#include "renderer/framebuffers.h"
#include "utils.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <vulkan/vulkan.h>

namespace Uriel {

void Framebuffer::setup(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
    this->instance = instance;
    this->physicalDevice = physicalDevice;
    this->device = device;
}

uint32_t Framebuffer::addAttachment(AttachmentCreateInfo& info) {
    AttachmentInfo attachment{};
    attachment.format = info.format;

    VkImageAspectFlags aspectMask{};

    if ((info.usageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0u) {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if ((info.usageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0u) {
        if (attachment.hasDepth()) {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (attachment.hasStencil()) {
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }

    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = info.format;
    imageCreateInfo.extent.width = info.width;
    imageCreateInfo.extent.height = info.height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = info.layerCount;
    imageCreateInfo.samples = info.sampleCount;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = info.usageFlags;

    VkResult res = vkCreateImage(device, &imageCreateInfo, nullptr, &attachment.image);
    assert(!res);

    //  TODO: Starting Here
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, attachment.image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &allocInfo, nullptr, &attachment.memory);
    vkBindImageMemory(device, attachment.image, attachment.memory, 0);

    attachment.subresourceRange = {};
    attachment.subresourceRange.aspectMask = aspectMask;
    attachment.subresourceRange.levelCount = 1;
    attachment.subresourceRange.layerCount = info.layerCount;

    //  INFO: Image view is a description of how to interpret a VkImage, like what part to access, what format to treat, what aspect etc.
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.viewType = (info.layerCount == 1) ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY; 
    imageViewCreateInfo.format = info.format;
    imageViewCreateInfo.subresourceRange = attachment.subresourceRange;
    imageViewCreateInfo.subresourceRange.aspectMask = (attachment.hasDepth()) ? VK_IMAGE_ASPECT_DEPTH_BIT : aspectMask;
    imageViewCreateInfo.image = attachment.image;

    vkCreateImageView(device, &imageViewCreateInfo, nullptr, &attachment.imageView);
    
    attachment.description = {};
    attachment.description.samples = info.sampleCount;
    attachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.description.format = attachment.format;
    attachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (attachment.hasDepth() || attachment.hasStencil()) {
        attachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    } else {
        attachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    attachments.push_back(attachment);

    return static_cast<uint32_t>(attachments.size() - 1);
}

void Framebuffer::createRenderPass() {
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    for (uint32_t i = 0; i < attachments.size(); i++) {
        attachmentDescriptions.push_back(attachments[i].description);
    }

    std::vector<VkAttachmentReference> colorReferences;
    VkAttachmentReference depthReference;
    bool hasDepth = false;
    bool hasColor = false;

    uint32_t attachmentIndex = 0;
    for (uint32_t i = 0; i < attachments.size(); i++) {
        if (attachments[i].isDepthStencil()) {
            depthReference.attachment = attachmentIndex;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            hasDepth = true;
        } else {
            colorReferences.push_back({ attachmentIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
            hasColor = true;
        }
        attachmentIndex++;
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if (hasColor) {
        subpass.colorAttachmentCount = (uint32_t)colorReferences.size();
        subpass.pColorAttachments = colorReferences.data();
    }
    if (hasDepth) {
        subpass.pDepthStencilAttachment = &depthReference;
    }

    std::array<VkSubpassDependency, 4> dependencies{};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;


    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dependencies[2].srcSubpass = 0;
    dependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    dependencies[3].srcSubpass = 0;
    dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[3].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[3].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[3].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[3].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;


    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 4;
    renderPassInfo.pDependencies = dependencies.data();
    
    vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_renderPass);

    std::vector<VkImageView> attachmentViews;
    for (auto& attachment : attachments) {
        attachmentViews.push_back(attachment.imageView);
    }

    uint32_t maxLayers = 0;
    for (auto& attachment : attachments) {
        maxLayers = std::max(attachment.subresourceRange.layerCount, maxLayers);
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_renderPass;
    framebufferInfo.pAttachments = attachmentViews.data();
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = maxLayers;
    vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_framebuffer);
}

}
