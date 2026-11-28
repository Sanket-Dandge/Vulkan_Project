#include "core/descriptors.h"
#include "helpers/logger.h"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace capella {
    void DescriptorSetLayout::create(VkDevice device, std::vector<VkDescriptorSetLayoutBinding> descriptorBindings) {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(descriptorBindings.size());
        layoutInfo.pBindings = descriptorBindings.data();

        if (!vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &mDescriptorSetLayout)) {
            LOG_ERROR(TAG, "Failed to create descriptor set layout");
            std::terminate();
        }
    }

    VkDescriptorSetLayoutBinding DescriptorSetLayout::addDescriptorBinding(VkDescriptorType descriptorType, VkDescriptorBindingFlags flags, uint32_t binding, uint32_t descriptorCount) {
        VkDescriptorSetLayoutBinding descriptorBinding{};
        descriptorBinding.binding = binding;
        descriptorBinding.descriptorType = descriptorType;
        descriptorBinding.stageFlags = flags;
        descriptorBinding.descriptorCount = descriptorCount;

        return descriptorBinding;
    }

    void DescriptorPool::create(VkDevice device, std::vector<VkDescriptorPoolSize> poolSizes, uint32_t maxConcurrentFrames) {
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxConcurrentFrames;

        if (!vkCreateDescriptorPool(device, &poolInfo, nullptr, &mDescriptorPool)) {
            LOG_ERROR(TAG, "Failed to allocate descriptor pool");
        }
    }

    VkDescriptorPoolSize DescriptorPool::addPoolSize(VkDescriptorType poolType, uint32_t descriptorCount) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = poolType;
        poolSize.descriptorCount = descriptorCount;

        return poolSize;
    }

    void DescriptorSet::create(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, std::vector<VkWriteDescriptorSet> writeDescriptors, uint32_t maxConcurrentFrames) {
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = descriptorPool;
        allocateInfo.pSetLayouts = &descriptorSetLayout;
        allocateInfo.descriptorSetCount = static_cast<uint32_t>(maxConcurrentFrames);

        for (int i = 0; i < maxConcurrentFrames; i++) {
            if (!vkAllocateDescriptorSets(device, &allocateInfo, &mDescriptorSet[i])) {
                LOG_ERROR(TAG, "Failed to allocate descriptor set");
            }

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptors.size()), writeDescriptors.data(), 0, nullptr);
        }
    }

    VkDescriptorImageInfo DescriptorSet::addDescriptorImage(VkImageLayout imageLayout, VkImageView imageView, VkSampler sampler) {
        VkDescriptorImageInfo descImgInfo{};
        descImgInfo.sampler = sampler;
        descImgInfo.imageView = imageView;
        descImgInfo.imageLayout = imageLayout;

        return descImgInfo;
    }

    VkWriteDescriptorSet DescriptorSet::writeDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType descriptorType, uint32_t binding, VkDescriptorBufferInfo bufferInfo, uint32_t descriptorCount) {
        VkWriteDescriptorSet descWriteInfo{};
        descWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWriteInfo.dstSet = dstSet;
        descWriteInfo.descriptorType = descriptorType;
        descWriteInfo.dstBinding = binding;
        descWriteInfo.pBufferInfo = &bufferInfo;
        descWriteInfo.descriptorCount = descriptorCount;

        return descWriteInfo;
    }

}  
