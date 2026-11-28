#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include <exception>
#include "helpers/logger.h"

namespace capella {
    class DescriptorSetLayout {
        public:
            void create(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> descriptorBindings);
            VkDescriptorSetLayoutBinding addDescriptorBinding(VkDescriptorType descriptorType, VkDescriptorBindingFlags flags, uint32_t binding, uint32_t descriptorCount = 1);

        private:
            VkDescriptorSetLayout mDescriptorSetLayout;
            const char* TAG = "DescriptorSetLayout";
    };

    class DescriptorPool {
        public:
            void create(VkDevice device, std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxConcurrentFrames);
            VkDescriptorPoolSize addPoolSize(VkDescriptorType poolType, uint32_t descriptorCount);

        private:
            VkDescriptorPool mDescriptorPool;
            const char* TAG = "DescriptorPool";
    };

    class DescriptorSet {
        public:
            void create(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VkWriteDescriptorSet> writeDescriptors, uint32_t maxConcurrentFrames);
            VkDescriptorImageInfo addDescriptorImage(VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler);
            VkWriteDescriptorSet writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType descriptorType, uint32_t binding, VkDescriptorBufferInfo bufferInfo, uint32_t descriptorCount);

        private:
            std::vector<VkDescriptorSet> mDescriptorSet;
            const char* TAG = "DescriptorSet";
    };
}


#endif
