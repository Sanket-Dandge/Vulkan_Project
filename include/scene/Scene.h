#ifndef SCENE_H
#define SCENE_H

#include "scene/MaterialAsset.h"
#include "scene/MeshAsset.h"
#include "scene/SceneNode.h"
#include "scene/SkinAndAnimationAsset.h"
#include "scene/TextureAsset.h"
#include <string>
#include <vector>

namespace capella {

struct Scene {
    std::vector<SceneNode> nodes;         //  @info: multiple nodes
    std::vector<MeshAsset> meshes;        //  @info: multiple meshes
    std::vector<MaterialAsset> materials; //  @info: multiple materials
    std::vector<TextureAsset> textures;   //  @info: multiple textures
    std::vector<uint32_t>
        rootIndices;         //  @info: root node indices in active gltf scene
    std::vector<Skin> skins; //  @info: multiple skins
    std::vector<Animation> animations; //  @info: multiple skins
    std::vector<Camera> cameras;       //  @info: multiple cameras

    void traverse(SceneNode visitor); //  @info: check if node has a mesh or not
                                      //  and letting renderer know
    std::string name;                 //  @info: Name of the scene
};

}

#endif
