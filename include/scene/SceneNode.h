#ifndef SCENENODE_H
#define SCENENODE_H


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#define INVALID_NODE_INDEX UINT32_MAX

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdint>
#include <string>

struct SceneNode {
    std::string name;   //  @info: Name of the SceneNode object
    uint32_t parentIndex = INVALID_NODE_INDEX;  //  @info: Root node (Scene)
    std::vector<uint32_t> childIndices; //  @info: vector of indices of node's children

    // glm::vec3 transform { 0.0f };
    // glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    // glm::vec3 scale { 1.0f };
    glm::mat4 localTransform = glm::mat4(1.0f);   //  @info: transform * rotation * scale
    glm::mat4 worldTransform = glm::mat4(1.0f);   //  @info: computed in single forward pass over sorted array

    //  @info: Scene Object References
    int32_t meshIndex = -1;
    int32_t cameraIndex = -1;
    int32_t skinIndex = -1;

    //  @info: Helper Functions
    bool hasMesh() { return meshIndex >= 0; }
    bool hasCamera() { return cameraIndex >= 0; }
    bool hasParent() { return parentIndex == INVALID_NODE_INDEX; }
};

struct CameraPerspective {
    float aspectRatio;
    float yfov;
    float zfar;
    float znear;
};

struct CameraOrthographic {
    float xmag;
    float ymag;
    float zfar;
    float znear;
};


struct Camera {
    std::string name;
    std::string type;
    CameraOrthographic orthographic{};
    CameraPerspective perspective{};
};

#endif
