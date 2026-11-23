#ifndef MATERIALASSET_H
#define MATERIALASSET_H

#include <string>
#include <vulkan/vulkan.h>
#include "glm/glm.hpp"

enum class AlphaMode : uint8_t {
    OPAQUE = 0,
    MASK,
    BLEND,
};

struct MaterialAsset {
    std::string name;
    glm::vec4 baseColorFactor{1.f};         
    float metallicFactor{1.f};              
    float roughnessFactor{1.f};             
    glm::vec3 emissiveFactor{0.f};          
    float alphaCutoff{0.5f};                
    AlphaMode alphaMode = AlphaMode::OPAQUE;
    bool doubleSided{false};

    int32_t baseColorTextureIndex{-1};
    int32_t metallicRoughnessTextureIndex{-1};
    int32_t normalTextureIndex{-1};
    int32_t occlusionTextureIndex{-1};
    int32_t emissiveTextureIndex{-1};

    uint32_t baseColorTextureTexCoord{0};
    uint32_t metallicRoughnessTextureTexCoord{0};
    uint32_t normalTextureTexCoord{0};
    uint32_t occlusionTextureTexCoord{0};
    uint32_t emissiveTextureTexCoord{0};

    float occlusionTextureStrength{1.f};
    float normalTextureScale{1.f};

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    bool isDoubleSided() { return doubleSided; }
};


#endif
