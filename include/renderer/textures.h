#pragma once

#include "core/context.h"
#include <string>
#include <vulkan/vulkan.h>
#include <ktx.h>
#include <ktxvulkan.h>

namespace capella {

class Texture {
    public:
        Context* ctx;
        uint32_t width;
        uint32_t height;
        VkDeviceSize imageSize;
        VkImage image;
        VkImageView imageView;
        VkSampler sampler;
        VkDescriptorImageInfo descriptor;
        VkImageLayout imageLayout;
        VkDeviceMemory deviceMemory;
        uint32_t mipLevels;
        
        ktxResult loadKTXFile(std::string filename, ktxTexture** target);
        void setup(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);

    protected:
        VkInstance instance;
        VkPhysicalDevice physicalDevice;
        VkDevice device;
};

}
