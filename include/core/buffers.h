#pragma once

#include <vulkan/vulkan.h>

namespace capella {

class Buffer {
    public:
        VkBuffer buffer{};
        VkDeviceMemory bufferMemory{};
        VkDeviceSize bufferSize{};
        VkBufferUsageFlags usage{};
        VkMemoryPropertyFlags properties{};
        void* bufferData = nullptr;
		VkDeviceSize alignment = 0;
		VkMemoryPropertyFlags memoryPropertyFlags;
		VkDescriptorBufferInfo descriptor;

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void copyMemory(void* data, VkDeviceSize size);
        void unmap();
        void destroy();
        VkResult bindBuffer(VkDeviceSize offset = 0);
		VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        //  TODO: Need functionality for these functions
        void createBuffer();
        void allocateMemoryForBuffer();

        //  TODO: Check if buffer initialization is needed in the setup function or not
        void setup(VkDevice device);
        VkDevice device{};

    private:
};

}
