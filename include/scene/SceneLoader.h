#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "core/context.h"
#include "scene/Scene.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tinygltf/tiny_gltf.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Uriel {

class SceneLoader {
    public:
        void buildSceneGraph(const tinygltf::Model& model, Scene& scene);

        void loadTextures();
        void loadMaterials();
        void loadMeshes(const tinygltf::Model& model, Scene& scene);

        //  @note: HELPER FUNCTIONS
        bool getAttributeDetails(const tinygltf::Model &model, const tinygltf::Primitive &primitive,
                                 const std::string &accessorName, const unsigned char *&bufferPointer,
                                 const tinygltf::Accessor *&accessorPointer, size_t &bufferStride);

      private:
        Context* ctx;
};

}

#endif
