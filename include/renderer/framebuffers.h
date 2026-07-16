#pragma once

#include <algorithm>
#include <iterator>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Uriel {

struct AttachmentInfo {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory memory;
    VkFormat format;
    VkAttachmentDescription description;
    VkImageSubresourceRange subresourceRange;

    [[nodiscard]] bool hasDepth() const {
        std::vector<VkFormat> formats = {
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
        };
        return std::find(formats.begin(), formats.end(), format) != std::end(formats);
    }

    [[nodiscard]] bool hasStencil() const {
        std::vector<VkFormat> formats = {
            VK_FORMAT_D16_UNORM_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
        };
        return std::find(formats.begin(), formats.end(), format) != std::end(formats);
    }

    bool isDepthStencil() {
        return(hasDepth() || hasStencil());
    }
};

struct AttachmentCreateInfo {
    uint32_t width;
    uint32_t height;
    uint32_t layerCount;
    VkFormat format;
    VkImageUsageFlags usageFlags;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
};

class Framebuffer {
    public:
        uint32_t width;
        uint32_t height;
        VkFramebuffer m_framebuffer;
        VkRenderPass m_renderPass;

		std::vector<AttachmentInfo> attachments;
        
        void setup(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
        void createRenderPass();
        
        uint32_t addAttachment(AttachmentCreateInfo& info);

    private:
        VkInstance instance{VK_NULL_HANDLE};
        VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
        VkDevice device{VK_NULL_HANDLE};
};

}
