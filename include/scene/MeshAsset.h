#ifndef MESHASSET_H
#define MESHASSET_H

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "glm/glm.hpp"

struct Vertex {
    //  @note: required attribute
    glm::vec3 position;

    //  @note: optional attributes
    glm::vec3 normal;
    glm::vec4 tangent;
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec4 color;
    glm::uvec4 joints;
    glm::vec4 weights;
};

//  TODO: Will need it in phase-3
struct AABB {
    glm::vec3 min{0.f};
    glm::vec3 max{0.f};
};

struct MeshPrimitive {
    VkBuffer vertexBuffer{VK_NULL_HANDLE};      //  @info: Vertex Buffer data                                
    VkDeviceSize vertexBufferOffset{0};    //  @info: offset for vertex attribute identification
    VkBuffer indexBuffer{VK_NULL_HANDLE};       //  @info: Index Buffer data                                 
    VkDeviceSize indexBufferOffset{0};    //  @info: offset for indices attribute identification
    uint32_t vertexCount{0};       //  @info: Total number of vertices                          
    uint32_t indexCount{0};        //  @info: Total number of indices                           
    int32_t materialIndex{-1};     //  @info: the material used on the mesh primitive            
                                   //
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
                               //
    //  TODO: Will need it in phase-3
    AABB boundingBox;           //  @info: world space for phase-3 culling
};

struct MeshAsset {
    std::string name;               
    std::vector<MeshPrimitive> primitives;
};


#endif
