#ifndef TEXTUREASSET_H
#define TEXTUREASSET_H

#include "core/context.h"
#include <vulkan/vulkan.h>
#include <tinygltf/tiny_gltf.h>

namespace capella {

struct TextureAsset {
    public:
        VkImage image;
        VkImageView imageview;
        VkSampler imagesampler;
        VkDeviceMemory imagememory;

        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t miplevels = 1;
        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

        void loadglTFImage(tinygltf::Image& gltfImage, std::string filepath, Context* ctx, VkQueue cmdQueue);

        //  WARN: Not sure about these functions
        void updateDescriptor();
        void destroy();

    private:
        void generateMipMaps(Context* ctx, VkQueue blitQueue);
        void createSamplers(Context* ctx);
};

}

#endif
