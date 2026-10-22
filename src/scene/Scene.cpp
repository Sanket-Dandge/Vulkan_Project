#include "scene/Scene.h"

void Scene::bindResourcesIfMeshExists(SceneNode node, glm::mat4 worldTransform) {

}

void Scene::traverse(SceneNode visitor) {
    for (const SceneNode& node : nodes) {
        if (node.meshIndex == -1) {
            continue;
        }
    }
}
