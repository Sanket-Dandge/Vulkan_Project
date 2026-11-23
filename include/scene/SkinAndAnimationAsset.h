#ifndef SKINANIMATION_H
#define SKINANIMATION_H

#include "helpers/logger.h"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <limits>
#include <string>
#include <vector>

struct Skin {
    std::string name;
    int32_t skeletonIndex = -1;
    std::vector<int32_t> jointIndices;
    std::vector<glm::mat4> inverseBindMatrices;
};


struct AnimationChannel {
    public:
        enum PathType {
            TRANSLATION,
            ROTATION,
            SCALE,
            WEIGHTS,
            UNKNOWN,
        };

        PathType pathType = PathType::UNKNOWN;
        int32_t samplerIndex = -1;
        uint32_t nodeIndex = -1;

        PathType stringToPath(std::string path) {
            if (path == "translation") {
                return PathType::TRANSLATION;
            } else if (path == "rotation") {
                return PathType::ROTATION;
            } else if (path == "scale") {
                return PathType::SCALE;
            } else if (path == "weights") {
                return PathType::WEIGHTS;
            } else {
                return PathType::UNKNOWN;
            } 
        }
};


struct AnimationSampler {
    enum InterpolationType {
        LINEAR,
        STEP,
        CUBICSPLINE
    };

    std::vector<int> inputs;
    std::vector<glm::vec4> outputs;
    InterpolationType interpolationType = InterpolationType::LINEAR;

    InterpolationType stringToPath(std::string path) {
        if (path == "step") {
            return InterpolationType::STEP;
        } else if (path == "cubicspline") {
            return InterpolationType::CUBICSPLINE;
        } else {
            return InterpolationType::LINEAR;
        } 
    }
};

struct Animation {
    std::string name;
    std::vector<AnimationChannel> channels;
    std::vector<AnimationSampler> samplers;

    float startKeyframe = std::numeric_limits<float>::max();
    float endKeyframe = std::numeric_limits<float>::max();

};


#endif
