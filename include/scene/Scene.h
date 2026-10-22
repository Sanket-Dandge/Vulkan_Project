#ifndef SCENE_H
#define SCENE_H

#include "scene/SceneNode.h"
#include "scene/MeshAsset.h"
#include "scene/MaterialAsset.h"
#include "scene/TextureAsset.h"
#include <string>
#include <vector>

struct Scene {
    std::vector<SceneNode> nodes;           //  @info: multiple nodes            
    std::vector<MeshAsset> meshes;          //  @info: multiple meshes          
    std::vector<MaterialAsset> materials;   //  @info: multiple materials
    std::vector<TextureAsset> textures;     //  @info: multiple textures   
    std::vector<uint32_t> rootIndices;      //  @info: root node indices in active gltf scene
                                            
    void traverse(SceneNode visitor);       //  @info: check if node has a mesh or not and letting renderer know
    void bindResourcesIfMeshExists(SceneNode node, glm::mat4 worldTransform);   //  @info: if mesh exists for a node, bind relevant resources
    std::string name;                       //  @info: Name of the scene
};


#endif
