#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <vulkan/vulkan_core.h>

namespace capella {

class Synchronization {
    public:
        void createFence();
        void createSemaphore();

    private:
        VkFence mFence;
        VkSemaphore mSemaphore;
};

}

#endif
