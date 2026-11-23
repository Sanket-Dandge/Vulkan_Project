#include "scene/SceneLoader.h"
#include "scene/MaterialAsset.h"
#include "scene/MeshAsset.h"
#include "scene/SceneNode.h"
#include "scene/TextureAsset.h"
#include <cstddef>
#include <cstring>
#include <queue>
#include <vector>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
// #define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <tinygltf/tiny_gltf.h>

namespace capella {

//  TODO: Need to add error handling
void SceneLoader::buildSceneGraph(const tinygltf::Model& model, Scene& scene) {
    //  @step-1: Allocate Node

    //  @info: reserving space for nodes of the model
    scene.nodes.reserve(model.nodes.size());

    //  @info: storing node info in SceneNode vector
    for (const auto& node : model.nodes) {
        const tinygltf::Node gltfNode = node;
        SceneNode scenenode;

        scenenode.name = gltfNode.name;
        scenenode.meshIndex = gltfNode.mesh;

        if (gltfNode.camera != -1) {
            scenenode.cameraIndex = gltfNode.camera;
        }

        if (gltfNode.skin != -1) {
            scenenode.skinIndex = gltfNode.skin;
        }

        //  @info: in case model's transform is not empty
        if (!gltfNode.matrix.empty()) {
            scenenode.localTransform = glm::make_mat4(gltfNode.matrix.data());
        } else {
            //  @info: in case it is empty
            //  @Here, info on translate, scale and rotate for each node is collected and at the end 
            //  @used for calculating local transform position
            glm::mat4 translate(1.f), scale(1.f), rotate(1.f);

            if (!gltfNode.translation.empty()) {
                translate = glm::translate(glm::mat4(1.f),
                    glm::vec3(gltfNode.translation[0], gltfNode.translation[1],
                              gltfNode.translation[2]));
            }

            if (!gltfNode.scale.empty()) {
                scale = glm::scale(glm::mat4(1.f),
                               glm::vec3(gltfNode.scale[0], gltfNode.scale[1],
                                         gltfNode.scale[2]));
            }

            if (!gltfNode.rotation.empty()) {
                //  @info: for rotation, it has quaternion order
                rotate = glm::mat4_cast(glm::quat(
                    (float)gltfNode.rotation[3], (float)gltfNode.rotation[0],
                    (float)gltfNode.rotation[1], (float)gltfNode.rotation[2]));
            }

            //  @info: Calculating local transform position
            scenenode.localTransform = translate * rotate * scale;
        }

        //  @info: Storing the node info in scene
        scene.nodes.push_back(scenenode);
    }

    //  @step-2: Wire up the children-parent relationship
    for (int i = 0; i < model.nodes.size(); i++) {
        for (int childIndex : model.nodes[i].children) {
            scene.nodes[i].childIndices.push_back(childIndex);  //  @info: node's children info gathering
            scene.nodes[childIndex].parentIndex = (uint32_t)i;  //  @info: assigning parent index to each children
        }
    }

    //  @info: storing indices that have no parent in rootIndices vector
    for (int i = 0; i < scene.nodes.size(); i++) {
        if (scene.nodes[i].parentIndex == INVALID_NODE_INDEX) {
            scene.rootIndices.push_back(i);
        }
    }

    //  @step-3: Topological sort

    //  @info: vector storing parent-before-child heirarchy indices
    std::vector<uint32_t> sortedIndices;
    sortedIndices.reserve(scene.nodes.size());

    //  @info: creating queue to establish a sequence
    std::queue<uint32_t> queue;
    for (const uint32_t& queueIndex : scene.rootIndices) {
        queue.push(queueIndex);
    }

    //  @info: looping through queue to push parent indices first and then child indices
    while (!queue.empty()) {
        uint32_t queueElement = queue.front();
        queue.pop();
        sortedIndices.push_back(queueElement);

        for (const uint32_t& childIndex : scene.nodes[queueElement].childIndices) {
            queue.push(childIndex);
        }
    }

    //  @step-4: Propogate world transform
    for (const uint32_t& nodeIndex : sortedIndices) {
        if (scene.nodes[nodeIndex].parentIndex == INVALID_NODE_INDEX) {
            //  @info: when node has no parent
            scene.nodes[nodeIndex].worldTransform = scene.nodes[nodeIndex].localTransform;
        } else {
            //  @info: when node has a parent
            scene.nodes[nodeIndex].worldTransform =
                scene.nodes[scene.nodes[nodeIndex].parentIndex].worldTransform *
                scene.nodes[nodeIndex].localTransform;
        }
    }
}

//  TODO: Check if more error handling is needed
//  TODO: Add Comments
void SceneLoader::loadMeshes(const tinygltf::Model& model, Scene& scene) {
    scene.meshes.reserve(model.meshes.size() + scene.meshes.size());
    for (const auto& mesh : model.meshes) {
        MeshAsset newMesh;
        newMesh.name = mesh.name;
        for (int j = 0; j < mesh.primitives.size(); j++) {
            tinygltf::Primitive newPrimitive = mesh.primitives[j];
            MeshPrimitive mPrimitive{};
            // mPrimitive = newMesh.primitives[j];
            mPrimitive.materialIndex = newPrimitive.material;


            // @note: indices attributes Detailing
            if (newPrimitive.indices >= 0) {
                const tinygltf::Accessor& idxAccessor = model.accessors[newPrimitive.indices];
                const tinygltf::BufferView& idxBufferView = model.bufferViews[idxAccessor.bufferView];
                const tinygltf::Buffer& idxBuffer = model.buffers[idxBufferView.buffer];

                const unsigned char* idxBufPtr = &idxBuffer.data[idxBufferView.byteOffset + idxAccessor.byteOffset];
                size_t idxStride = idxAccessor.ByteStride(idxBufferView);

                mPrimitive.indexCount = idxAccessor.count;

                mPrimitive.indices.resize(mPrimitive.indexCount);
                for (int i = 0; i < idxAccessor.count; i++) {
                    const unsigned char* pData = idxBufPtr + (i * idxStride);
                    if (idxAccessor.type == TINYGLTF_TYPE_SCALAR) {
                        if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                            mPrimitive.indices[i] = *reinterpret_cast<const uint16_t*>(pData);
                        } else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                            mPrimitive.indices[i] = *reinterpret_cast<const uint32_t*>(pData);
                        } else if (idxAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                            mPrimitive.indices[i] = *reinterpret_cast<const uint8_t*>(pData);
                        } else if (idxAccessor.componentType !=
                                       TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE || idxAccessor.componentType !=
                                       TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || idxAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                            LOG_ERROR(TAG, "Index Component Type must be unsigned byte/short/int");
                        }
                    } else {
                        LOG_ERROR(TAG, "Index Type must be scalar");
                    }
                }
            }

            //  @note: Vertex Attributes Detailing
            const unsigned char *posBufPtr = nullptr, *norBufPtr = nullptr, *uvBufPtr = nullptr, *tanBufPtr = nullptr, *colBufPtr = nullptr, *jntBufPtr = nullptr, *wghBufPtr;
            const tinygltf::Accessor *posAccessor = nullptr, *norAccessor = nullptr, *uvAccessor = nullptr, *tanAccessor = nullptr, *colAccessor = nullptr, *jntAccessor = nullptr, *wghAccessor = nullptr;
            size_t posStride = 0, norStride = 0, uvStride = 0, tanStride = 0, colStride = 0, jntStride = 0, wghStride = 0;

            if (!getAttributeDetails(model, newPrimitive, "POSITION", posBufPtr, posAccessor, posStride)) {
                continue;
            }

            mPrimitive.vertexCount = posAccessor->count;
            mPrimitive.boundingBox.max = glm::vec3(posAccessor->maxValues[0], posAccessor->maxValues[1], posAccessor->maxValues[2]);
            mPrimitive.boundingBox.min = glm::vec3(posAccessor->minValues[0], posAccessor->minValues[1], posAccessor->minValues[2]);

            bool hasNormalDetails = getAttributeDetails(model, newPrimitive, "NORMAL", norBufPtr, norAccessor, norStride);
            bool hasUVDetails = getAttributeDetails(model, newPrimitive, "TEXCOORD_0", norBufPtr, uvAccessor, uvStride);
            bool hasTangentDetails = getAttributeDetails(model, newPrimitive, "TANGENT", uvBufPtr, uvAccessor, tanStride);
            bool hasColorDetails = getAttributeDetails(model, newPrimitive, "COLOR_0", colBufPtr, colAccessor, colStride);
            bool hasJointsDetails = getAttributeDetails(model, newPrimitive, "JOINTS_0", jntBufPtr, jntAccessor, jntStride);
            bool hasWeightDetails = getAttributeDetails(model, newPrimitive, "WEIGHTS_0", wghBufPtr, wghAccessor, wghStride);

            mPrimitive.vertices.resize(mPrimitive.vertexCount);
            for (int i = 0; i < mPrimitive.vertexCount; i++) {
                Vertex& vertex = mPrimitive.vertices[i];

                //  @info: position vector storing
                if (posAccessor->type == TINYGLTF_TYPE_VEC3 && posAccessor->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    const float* pData = reinterpret_cast<const float*>(posBufPtr + i * posStride);
                    vertex.position = glm::vec3(pData[0], pData[1], pData[2]);
                } else if (posAccessor->type != TINYGLTF_TYPE_VEC3 ) {
                    LOG_ERROR(TAG, "Position type needs to be vec3");
                } else if (posAccessor->componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    LOG_ERROR(TAG, "Position Component Type must be float");
                }

                //  @info: normal vector storing
                if (hasNormalDetails) {
                    if (norAccessor->type == TINYGLTF_TYPE_VEC3 && norAccessor->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        const float* pData = reinterpret_cast<const float*>(norBufPtr + i * norStride);
                        vertex.normal = glm::vec3(pData[0], pData[1], pData[2]);
                    } else if (norAccessor->type != TINYGLTF_TYPE_VEC3 ) {
                        LOG_ERROR(TAG, "Normal type needs to be vec3");
                    } else if (norAccessor->componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        LOG_ERROR(TAG, "Normal Component Type must be float");
                    }
                } else {
                    vertex.normal = glm::vec3(0.f, 1.0f, 0.f);
                }

                //  @info: texture coordinates vector storing
                if (hasUVDetails) {
                    if (uvAccessor->type == TINYGLTF_TYPE_VEC2 && uvAccessor->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        const float* pData = reinterpret_cast<const float*>(uvBufPtr + i * uvStride);
                        vertex.uv0 = glm::vec2(pData[0], pData[1]);
                    } else if (uvAccessor->type != TINYGLTF_TYPE_VEC2 ) {
                        LOG_ERROR(TAG, "Texture Coordinate type needs to be vec2");
                    } else if (uvAccessor->componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        LOG_ERROR(TAG, "Texture Coordinate Component Type must be float");
                    }
                } else {
                    vertex.uv0 = glm::vec2(0.f, 0.f);
                }

                //  @info: tangent vector storing
                if (hasTangentDetails) {
                    if (tanAccessor->type == TINYGLTF_TYPE_VEC4 && tanAccessor->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        const float* pData = reinterpret_cast<const float*>(tanBufPtr + i * tanStride);
                        vertex.tangent = glm::vec4(pData[0], pData[1], pData[2], pData[3]);
                    } else if (tanAccessor->type != TINYGLTF_TYPE_VEC4) {
                        LOG_ERROR(TAG, "Tangent type needs to be vec4");
                    } else if (tanAccessor->componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        LOG_ERROR(TAG, "Tangent Component type needs to be float");
                    }
                } else {
                    vertex.tangent = glm::vec4(0.f, 0.f, 0.f, 1.f);
                }

                //  @info: color vector storing
                if (hasColorDetails) {
                    if (colAccessor->type == TINYGLTF_TYPE_VEC4 && colAccessor->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        const float* pData = reinterpret_cast<const float*>(colBufPtr + i * colStride);
                        vertex.color = glm::vec4(pData[0], pData[1], pData[2], pData[3]);
                    } else if (colAccessor->type == TINYGLTF_TYPE_VEC3) {
                        const float* pData = reinterpret_cast<const float*>(colBufPtr + i * colStride);
                        vertex.color = glm::vec4(pData[0], pData[1], pData[2], 1.0f);
                    } else if (colAccessor->type != TINYGLTF_TYPE_VEC4) {
                        LOG_ERROR(TAG, "Color type needs to be vec4");
                    } else if (colAccessor->componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        LOG_ERROR(TAG, "Color Component Type must be float");
                    }
                } else {
                    vertex.color = glm::vec4(1.f, 1.f, 1.f, 1.f);
                }

                //  @info: joints vector storing
                if (hasJointsDetails) {
                    if (jntAccessor->type == TINYGLTF_TYPE_VEC4 ) {
                        const float* pData = reinterpret_cast<const float*>(jntBufPtr + i * jntStride);
                        if (jntAccessor->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                            const uint16_t* u16 = reinterpret_cast<const uint16_t*>(pData);
                            vertex.joints = glm::uvec4(u16[0], u16[1], u16[2], u16[3]);
                        } else if (jntAccessor->componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                            vertex.joints = glm::vec4(pData[0], pData[1], pData[2], pData[3]);
                        }
                    } else if (jntAccessor->type != TINYGLTF_TYPE_VEC4) {
                        LOG_ERROR(TAG, "Joint type needs to be vec4");
                    } else if (jntAccessor->componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || jntAccessor->componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        LOG_ERROR(TAG, "Joint Component Type must be unsigned byte or unsigned short");
                    }
                } else {
                    vertex.joints = glm::vec4(0.f);
                }

                //  @info: weight vector storing
                if (hasWeightDetails) {
                    if (wghAccessor->type == TINYGLTF_TYPE_VEC4 && wghAccessor->componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        const float* pData = reinterpret_cast<const float*>(wghBufPtr + i * wghStride);
                        vertex.tangent = glm::vec4(pData[0], pData[1], pData[2], pData[3]);
                    } else if (wghAccessor->type != TINYGLTF_TYPE_VEC4) {
                        LOG_ERROR(TAG, "Weight type needs to be vec4");
                    } else if (wghAccessor->componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        LOG_ERROR(TAG, "Weight Component Type must be float");
                    }
                } else {
                    vertex.weights = glm::vec4(0.f);
                }
            }

            newMesh.primitives.push_back(mPrimitive);
        }
        scene.meshes.push_back(newMesh);
    }
}

//  @note: load material details
void SceneLoader::loadMaterials(const tinygltf::Model& model, Scene& scene) {
    scene.materials.reserve(scene.materials.size() + model.materials.size());
    for (auto material : model.materials) {
        MaterialAsset newMaterial;
        newMaterial.name = material.name;

        newMaterial.baseColorFactor = glm::vec4(material.pbrMetallicRoughness.baseColorFactor[0], material.pbrMetallicRoughness.baseColorFactor[1],
                      material.pbrMetallicRoughness.baseColorFactor[2], material.pbrMetallicRoughness.baseColorFactor[3]);
        newMaterial.baseColorTextureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
        newMaterial.baseColorTextureTexCoord = material.pbrMetallicRoughness.baseColorTexture.texCoord;

        newMaterial.emissiveFactor = glm::vec3(material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2]);
        newMaterial.emissiveTextureIndex = material.emissiveTexture.index;
        newMaterial.emissiveTextureTexCoord = material.emissiveTexture.texCoord;

        newMaterial.metallicFactor = material.pbrMetallicRoughness.metallicFactor;
        newMaterial.roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
        newMaterial.metallicRoughnessTextureIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
        newMaterial.metallicRoughnessTextureTexCoord = material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

        newMaterial.normalTextureIndex = material.normalTexture.index;
        newMaterial.normalTextureTexCoord = material.normalTexture.texCoord;
        newMaterial.normalTextureScale = material.normalTexture.scale;

        newMaterial.occlusionTextureIndex = material.occlusionTexture.index;
        newMaterial.occlusionTextureTexCoord = material.occlusionTexture.texCoord;
        newMaterial.occlusionTextureStrength = material.occlusionTexture.strength;

        if (material.alphaMode == "MASK") {
            newMaterial.alphaMode = AlphaMode::MASK;
        } else if (material.alphaMode == "BLEND") {
            newMaterial.alphaMode = AlphaMode::BLEND;
        } else {
            newMaterial.alphaMode = AlphaMode::OPAQUE;
        }

        newMaterial.doubleSided = material.doubleSided;

        scene.materials.push_back(newMaterial);
    }
}

//  #info: this function loads skins (OPTIONAL)
void SceneLoader::loadSkins(const tinygltf::Model& model, Scene& scene) {
    for (auto skin : model.skins) {
        Skin newSkin{};
        newSkin.name = skin.name;
        newSkin.skeletonIndex = skin.skeleton;
        for (auto joint : skin.joints) {
            newSkin.jointIndices.push_back(joint);
        }

        if (skin.inverseBindMatrices > -1) {
            const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
            const tinygltf::BufferView &bufferview = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = model.buffers[bufferview.buffer];
			memcpy(newSkin.inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferview.byteOffset], accessor.count * sizeof(glm::mat4));
        }
        scene.skins.push_back(newSkin);
    }
}

//  @info: this function loads animations (OPTIONAL)
void SceneLoader::loadAnimations(const tinygltf::Model& model, Scene& scene) {
    //  @info: loading info of all animations in the scene
    for (auto animation : model.animations) {
        Animation newAnimation{};
        newAnimation.name = animation.name;
        for (auto channel : animation.channels) {
            AnimationChannel newChannel{};
            newChannel.samplerIndex = channel.sampler;
            newChannel.nodeIndex = channel.target_node;
            newChannel.pathType = newChannel.stringToPath(channel.target_path);
            newAnimation.channels.push_back(newChannel);
        }

        //  @info: Reading and storing sampler time values
        for (auto sampler : animation.samplers) {
            AnimationSampler newSampler{};
            newSampler.interpolationType = newSampler.stringToPath(sampler.interpolation);

            const tinygltf::Accessor inAccessor = model.accessors[sampler.input];
            const tinygltf::BufferView inBufferView = model.bufferViews[inAccessor.bufferView];
            const tinygltf::Buffer inBuffer = model.buffers[inBufferView.buffer];

            const float* inBufPtr = reinterpret_cast<const float*>(&inBuffer.data[inBufferView.byteOffset + inAccessor.byteOffset]);
            size_t inBufStride = inAccessor.ByteStride(inBufferView);

            //  @info: storing inputs of keyframe times
            for (size_t index = 0; index < inAccessor.count; index++) {
                newSampler.inputs.push_back(inBufPtr[index]);
            }

            if (inAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                LOG_ERROR(TAG, "inAccessor componentType should be 'TINYGLTF_COMPONENT_TYPE_FLOAT'");
                return;
            }

            //  @info: store the start and end of the animation sequence
            for (auto input : newSampler.inputs) {
                if (input < newAnimation.startKeyframe) {
                    newAnimation.startKeyframe = input;
                } 
                if (input > newAnimation.endKeyframe) {
                    newAnimation.endKeyframe = input;
                }
            }

            //  @info: storing outputs of keyframe values
            const tinygltf::Accessor outAccessor = model.accessors[sampler.output];
            const tinygltf::BufferView outBufferView = model.bufferViews[outAccessor.bufferView];
            const tinygltf::Buffer outBuffer = model.buffers[outBufferView.buffer];

            if (outAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                LOG_ERROR(TAG, "outAccessor componentType should be 'TINYGLTF_COMPONENT_TYPE_FLOAT'");
                return;
            }

            if (outAccessor.type == TINYGLTF_TYPE_VEC3) {
                const glm::vec3* outBufPtr = reinterpret_cast<const glm::vec3*>(&outBuffer.data[outBufferView.byteOffset + outAccessor.byteOffset]);
                size_t outBufStride = outAccessor.ByteStride(outBufferView);
                for (size_t idx = 0; idx < outAccessor.count; idx++) {
                    newSampler.outputs.push_back(glm::vec4(outBufPtr[idx], 0.f));
                }
            } else if (outAccessor.type == TINYGLTF_TYPE_VEC4) {
                const glm::vec4* outBufPtr = reinterpret_cast<const glm::vec4*>(&outBuffer.data[outBufferView.byteOffset + outAccessor.byteOffset]);
                size_t outBufStride = outAccessor.ByteStride(outBufferView);
                for (size_t idx = 0; idx < outAccessor.count; idx++) {
                    newSampler.outputs.push_back(outBufPtr[idx]);
                }
            } else {
                LOG_ERROR(TAG, "Unknown outAccessor.types");
                return;
            }
            newAnimation.samplers.push_back(newSampler);
        }
        scene.animations.push_back(newAnimation);
    }
}

void SceneLoader::loadCamera(const tinygltf::Model& model, Scene& scene) {
    for (auto camera : model.cameras) {
        Camera newCamera{};
        newCamera.name = camera.name;
        newCamera.type = camera.type;
        if (camera.type == "Orthographic") {
            newCamera.orthographic.xmag = camera.orthographic.xmag;
            newCamera.orthographic.ymag = camera.orthographic.ymag;
            newCamera.orthographic.zfar = camera.orthographic.zfar;
            newCamera.orthographic.znear = camera.orthographic.znear;
        } else {
            newCamera.perspective.aspectRatio = camera.perspective.aspectRatio;
            newCamera.perspective.yfov = camera.perspective.yfov;
            newCamera.perspective.zfar = camera.perspective.zfar;
            newCamera.perspective.znear = camera.perspective.znear;
        }
        scene.cameras.push_back(newCamera);
    }

    //  TODO: Check if updating descriptor sets or framebuffer is required here
}

//  @info: this function loads the images/textures of the gltf model
void SceneLoader::loadImages(const tinygltf::Model& model, Scene& scene, VkQueue queue) {
    for (auto image : model.images) {
        TextureAsset texture{};
        texture.loadglTFImage(image, path, ctx, queue);
        scene.textures.push_back(texture);
    }

    //  TODO: Add createTextureIfEmtpy function in here
}

//  @note: HELPER FUNCTION
//  this function maps accessor,bufferview,buffer and store data and stride associated with it
bool SceneLoader::getAttributeDetails(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const std::string& accessorName, const unsigned char*& bufferPointer, const tinygltf::Accessor*& accessorPointer, size_t& bufferStride) {
    auto findAttribute = primitive.attributes.find(accessorName);
    if (findAttribute == primitive.attributes.end()) {
        return false;
    }

    accessorPointer = &model.accessors[findAttribute->second];
    const tinygltf::BufferView& bufferView = model.bufferViews[accessorPointer->bufferView];
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    bufferPointer = &buffer.data[bufferView.byteOffset + accessorPointer->byteOffset];
    bufferStride = accessorPointer->ByteStride(bufferView);

    return true;
}


//  @info: the function brings all the components of teh gltf together and load it
void SceneLoader::loadFromFile(std::string path) {
    tinygltf::Model model;
    tinygltf::TinyGLTF gltfContext;
    std::string err, warn;
    bool isFileLoaded = gltfContext.LoadASCIIFromFile(&model, &err, &warn, path);

    if (isFileLoaded) {
        buildSceneGraph(model, scene);
    }
}

}
