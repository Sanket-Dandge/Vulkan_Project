#ifndef TEXTUREASSET_H
#define TEXTUREASSET_H

#include <vulkan/vulkan.h>

struct TextureAsset {
    VkImage image;
    VkImageView imageview;
    VkSampler imagesampler;
    VkDeviceMemory imagememory;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t miplevels = 1;
    VkFormat format = VK_FORMAT_UNDEFINED;

    void updateDescriptor();
    void destroy();
};


#endif
