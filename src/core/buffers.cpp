#include "core/buffers.h"
#include <vulkan/vulkan.h>

namespace Uriel {

void Buffer::setup(VkDevice device) {
    this->device = device;
}

void Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
    vkMapMemory(device, bufferMemory, offset, bufferSize, 0, &bufferData);
}

void Buffer::copyMemory(void* data, VkDeviceSize size) {
    memcpy(bufferData, data, size);
}

void Buffer::unmap() {
    vkUnmapMemory(device, bufferMemory);
}

void Buffer::destroy() {
    if (buffer != nullptr) {
        vkDestroyBuffer(device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }
    
    if (bufferMemory != nullptr) {
        vkFreeMemory(device, bufferMemory, nullptr);
        bufferMemory = VK_NULL_HANDLE;
    }
}

VkResult Buffer::bindBuffer(VkDeviceSize offset) {
    return vkBindBufferMemory(device, buffer, bufferMemory, offset);
}

VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = bufferMemory,
        .offset = offset,
        .size = size
    };
    return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
}

VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange{
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = bufferMemory,
        .offset = offset,
        .size = size
    };
    return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
}

void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
    descriptor.offset = offset;
    descriptor.buffer = buffer;
    descriptor.range = size;
}

}
