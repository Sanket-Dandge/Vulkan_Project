#include "scene/Scene.h"

namespace capella {

void Scene::traverse(SceneNode visitor) {
    for (const SceneNode& node : nodes) {
        if (node.meshIndex == -1) {
            continue;
        }
    }
}

}
