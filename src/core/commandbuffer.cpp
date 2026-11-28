#include "core/commandbuffer.h"
#include "helpers/logger.h"
#include <exception>
#include <vulkan/vulkan_core.h>

namespace capella {
    void CommandPool::create(VkDevice& device) {
        mDevice = device;
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        if (!vkCreateCommandPool(device, &poolInfo, nullptr, &mCommandPool)) {
            LOG_ERROR(TAG, "Failed to create command pool.");
            std::terminate();
        }

        LOG_INFO(TAG, "Successfully created command pool.");
    }

    void CommandPool::destroy() {
        if (mCommandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
        }
    }

    void CommandBuffer::create(VkDevice& device, VkCommandPool& cmdPool, VkCommandBufferLevel bufferLevel, uint32_t maxFramesInFlight) {
        mCommandBuffers.resize(maxFramesInFlight);
        VkCommandBufferAllocateInfo bufferAllocInfo{};
        bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferAllocInfo.commandPool = cmdPool;
        bufferAllocInfo.level = bufferLevel;
        bufferAllocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

        if (!vkAllocateCommandBuffers(device, &bufferAllocInfo, mCommandBuffers.data())) {
            LOG_ERROR(TAG, "Failed to allocate command buffers.");
            std::terminate();
        }
    }

    VkCommandBuffer CommandBuffer::beginCmdBuffer(VkDevice& device, VkCommandPool& cmdPool, VkCommandBufferLevel bufferLevel, uint32_t maxFramesInFlight) {
        VkCommandBufferAllocateInfo bufferAllocInfo{};
        bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferAllocInfo.commandPool = cmdPool;
        bufferAllocInfo.level = bufferLevel;
        bufferAllocInfo.commandBufferCount = 1;

        VkCommandBuffer cmdBuffer;
        if (!vkAllocateCommandBuffers(device, &bufferAllocInfo, &cmdBuffer)) {
            LOG_ERROR(TAG, "Failed to begin command buffer.");
            std::terminate();
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &beginInfo);
        return cmdBuffer;
    }

    void CommandBuffer::endCmdBuffer(VkDevice device, VkCommandBuffer cmdBuffer, VkCommandPool cmdPool, VkQueue queue) {
        vkEndCommandBuffer(cmdBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuffer);
    }

}
