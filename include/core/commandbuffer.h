#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <exception>
#include "helpers/logger.h"

namespace capella {
    class CommandPool {
        public:
            void create(VkDevice& device);
            void destroy();

            uint32_t queueFamilyIndex = 0;
            
        private:
            VkCommandPool mCommandPool;

            VkDevice mDevice;
            const char* TAG = "CommandPool";

    };

    class CommandBuffer {
        public:
            void create(VkDevice& device, VkCommandPool& cmdPool, VkCommandBufferLevel bufferLevel, uint32_t maxFramesInFlight);
            VkCommandBuffer beginCmdBuffer(VkDevice& device, VkCommandPool& cmdPool, VkCommandBufferLevel bufferLevel, uint32_t maxFramesInFlight);
            void endCmdBuffer(VkDevice device, VkCommandBuffer cmdBuffer, VkCommandPool cmdPool, VkQueue queue);

        private:
            std::vector<VkCommandBuffer> mCommandBuffers;
            const char* TAG = "CommandBuffer";
    };
}

#endif
