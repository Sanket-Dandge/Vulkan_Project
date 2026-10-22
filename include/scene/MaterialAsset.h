#ifndef MATERIALASSET_H
#define MATERIALASSET_H

#include <vulkan/vulkan.h>
#include "glm/glm.hpp"

enum class AlphaMode : uint8_t {
    OPAQUE = 0,
    MASK,
    BLEND,
};

struct MaterialAsset {
    glm::vec4 baseColorFactor{1.f};         
    float metallicFactor{1.f};              
    float roughnessFactor{1.f};             
    glm::vec3 emissiveFactor{0.f};          
    float alphaCutoff{0.5f};                
    AlphaMode alphaMode = AlphaMode::OPAQUE;

    int32_t baseColorTextureIndex{-1};
    int32_t metallicRoughnessTextureIndex{-1};
    int32_t normalTextureIndex{-1};
    int32_t occlusionTextureIndex{-1};
    int32_t emissiveTextureIndex{-1};

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};


#endif
