#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <vulkan/vulkan.h>

namespace capella {

class Descriptos {
    public:
        void createDescriptorSetLayout();
        void createDescriptorSets();
        void createDescriptorPool();
        void createDescriptorPushConstants();
        
        VkDescriptorPool mDescriptorPool;
        VkDescriptorSetLayout mDescriptorSetLayout;
        VkDescriptorSet mDescriptorSets;

};

}


#endif
