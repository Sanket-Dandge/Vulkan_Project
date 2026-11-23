#include <cassert>
#include <vulkan/vulkan.h>
#include <cstring>

#include "core/context.h"
#include "core/Core.h"
#include "utils.h"

namespace capella {

/*
 *  @begin this function creates vulkan instance (which is connection between application & vulkan runtime).
 *  it also enables validation layer that intercepts API calls to check for errors.
 *  VkApplicationInfo defines the api version used for the application.
 *  it also adds layers and extensions as needed and are also checked to confirm availability.
 * */
void Context::initializeInstance(ContextCreateInfo& info) {
    //  @info: Validation Layer enable/disable
    if (info.enableValidationLayer) {
        info.addInstanceLayer("VK_LAYER_KHRONOS_validation", false);
    }

    //  @info: Defining the vulkan application
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pEngineName = info.engineName;
    applicationInfo.pApplicationName = info.appName;
    applicationInfo.apiVersion = VK_MAKE_VERSION(info.apiMajor, info.apiMinor, info.apiPatch);

    //  @info: Check if required instance layers and extensions are available or not
    auto layerProperties = getInstanceLayers();
    if (!checkInstanceEnablesArray(layerProperties, info.instanceLayers, info.m_availableInstanceLayers)) {
        return;
    }

    auto extensionProperties = getInstanceExtensions();
    if (!checkInstanceEnablesArray(extensionProperties, info.instanceExtensions, info.m_availableInstanceExtensions)) {
        return;
    }

    //  @info: Putting layers and extensions in new vectors for the vulkan instance creation info
    std::vector<const char*> instanceLayers;
    std::vector<const char*> instanceExtensions;

    instanceLayers.reserve(info.m_availableInstanceLayers.size());
    for (const auto& layer : info.m_availableInstanceLayers) {
        instanceLayers.push_back(layer);
    }

    instanceExtensions.reserve(info.m_availableInstanceExtensions.size());
    for (const auto& extension : info.m_availableInstanceExtensions) {
        instanceExtensions.push_back(extension);
    }

    //  @info: Instance Creation Info for creating vulkan instance object
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo        = &applicationInfo;
    instanceCreateInfo.enabledLayerCount       = static_cast<uint32_t>(instanceLayers.size());
    instanceCreateInfo.ppEnabledLayerNames     = instanceLayers.data();
    instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(instanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.flags                   = 0;
    instanceCreateInfo.pNext                   = nullptr;

    VkResult err = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
    assert(!err);

    LOG_INIT("Instance");

    //  @info: Condition To Enable Debugging
    for(const auto& extension : instanceExtensions) {
        if (strcmp(extension, "VK_EXT_debug_utils") == 0) {
            enableDebugging();
        }
    }
}

void Context::enableDebugging() {
    m_createDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
    m_destroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");

    if (m_createDebugUtilsMessengerEXT != nullptr) {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        addDebugMessengerCreateInfo(debugCreateInfo);

        VkResult err = m_createDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger);
        assert(!err);
    }

    LOG_INIT("Debug Messenger");
}

void ContextCreateInfo::setVersion(uint32_t major, uint32_t minor) {
    apiMajor = major;
    apiMinor = minor;
}

/*
 *  @begin this function is used to choose the physical device we want the vulkan application to run on.
 *  it also sets the device properties, device features and memory properties that are needed in the physical device
 *  also sets the queue family properties and device extensions and check if required device extensions are available or not.
 * */
void Context::pickPhysicalDevice(ContextCreateInfo& info, uint32_t index) {
    uint32_t gpuCount{};
    VkResult res = vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);
    assert(!res);

	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	res = vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());
    assert(!res);

    m_physicalDevice = physicalDevices[index];

    //  @info: Setting properties and features.
    vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
    vkGetPhysicalDeviceFeatures(m_physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    assert(queueFamilyCount > 0);
    m_queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

    auto extensions = getDeviceExtensions(m_physicalDevice);

    if (!checkInstanceEnablesArray(extensions, info.deviceExtensions, info.m_availableDeviceExtensions)) {
        return;
    }

    LOG_INIT("Physical Device");
}

/*
 *  @begin this function creates VkDevice which is the application interface that handles the working of physical device.
 *  it creates a VkDeviceQueueCreateInfo based on requested queues and connect them in the VkDeviceCreateInfo information.
 *  then queue is created by vkGetDeviceQueue of graphics, compute and transfer queue.
 *  Then we create command pool object which allocates memory for command buffer.
 *
 * */
void Context::createLogicalDevice(ContextCreateInfo& info, VkPhysicalDeviceFeatures requiredFeatures) {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    const float defaultQueuePriority(0.f);

    //  @info: Ensuring a queue is used
    if (info.requestedQueues == 0u) {
        DEBUGLOG(ERROR, "not requested queue flag, fill requestedQueues");
        return;
    }

    if (info.requestedQueues & VK_QUEUE_GRAPHICS_BIT) {
        info.queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = info.queueFamilyIndices.graphics;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &defaultQueuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    } else {
        info.queueFamilyIndices.graphics = 0;
    }


    if (info.requestedQueues & VK_QUEUE_COMPUTE_BIT) {
        info.queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
        if (info.queueFamilyIndices.compute != info.queueFamilyIndices.graphics) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = info.queueFamilyIndices.compute;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &defaultQueuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }
    } else {
        info.queueFamilyIndices.compute = info.queueFamilyIndices.graphics;
    }

    if (info.requestedQueues & VK_QUEUE_TRANSFER_BIT) {
        info.queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
        if ((info.queueFamilyIndices.transfer != info.queueFamilyIndices.graphics) && (info.queueFamilyIndices.transfer != info.queueFamilyIndices.compute)) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = info.queueFamilyIndices.transfer;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &defaultQueuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }
    } else {
        info.queueFamilyIndices.transfer = info.queueFamilyIndices.graphics;
    }

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    deviceCreateInfo.pEnabledFeatures = &requiredFeatures;

    deviceCreateInfo.enabledExtensionCount = (uint32_t)info.m_availableDeviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = info.m_availableDeviceExtensions.data();

    VkResult res = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
    assert(!res);

    vkGetDeviceQueue(m_device, info.queueFamilyIndices.graphics, 0, &m_graphicsQueue);

    if (info.queueFamilyIndices.compute != info.queueFamilyIndices.graphics) {
        vkGetDeviceQueue(m_device, info.queueFamilyIndices.graphics, 0, &m_computeQueue);
    } else {
        m_computeQueue = m_graphicsQueue;
    }

    if (info.queueFamilyIndices.transfer != info.queueFamilyIndices.graphics && info.queueFamilyIndices.transfer != info.queueFamilyIndices.compute) {
        vkGetDeviceQueue(m_device, info.queueFamilyIndices.transfer, 0, &m_transferQueue);
    } else {
        m_transferQueue = m_graphicsQueue;
    }

    m_commandPool = createCommandPool(info.queueFamilyIndices.graphics);
    queueIndex = info.queueFamilyIndices.graphics;

    LOG_INIT("Logical Device");
}

/*
 *  @begin this function creates command pool object that allocates memory to the command buffer.
 * */
VkCommandPool Context::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags) {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = createFlags;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    VkCommandPool commandPool{};
    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    return commandPool;
}

/*
 *  @begin this function returns all the instance layers that are available on the system
 * */
std::vector<VkLayerProperties> Context::getInstanceLayers() {
    uint32_t layerCount = 0;
    VkResult err = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    assert(!err);

    std::vector<VkLayerProperties> layerProperties;
    layerProperties.resize(layerCount);
    err = vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
    assert(!err);
    return layerProperties;
}

/*
 *  @begin this function returns all the instance extensions that are available on the system
 * */
std::vector<VkExtensionProperties> Context::getInstanceExtensions() {
    uint32_t extensionCount = 0;
    VkResult err = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    assert(!err);
    std::vector<VkExtensionProperties> extensionProperties;
    extensionProperties.resize(extensionCount);

    err = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());
    assert(!err);
    return extensionProperties;
}

/*
 *  @begin this function returns all the device extensions that are available on the system
 * */
std::vector<VkExtensionProperties> Context::getDeviceExtensions(VkPhysicalDevice physicalDevice) {
    uint32_t count = 0;
    std::vector<VkExtensionProperties> extensionProperties;
    VkResult err = (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr));
    assert(!err);

    extensionProperties.resize(count);
    err = (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensionProperties.data()));
    assert(!err);
    extensionProperties.resize(std::min(extensionProperties.size(), size_t(count)));
    return extensionProperties;
}

/*
 *  @begin this function adds the required layer for usage in the instance object
 * */
void ContextCreateInfo::addInstanceLayer(const char* layerName, bool optional /* = false */) {
    instanceLayers.emplace_back(layerName, optional);
}

/*
 *  @begin this function adds the required extension for usage in the instance object
 * */
void ContextCreateInfo::addInstanceExtension(const char* extensionName, bool optional /* = false */) {
    instanceExtensions.emplace_back(extensionName, optional);
}

/*
 *  @begin this function adds the required extension for usage in the device creation
 * */
void ContextCreateInfo::addDeviceExtension(const char* extension, bool optional /* = false */) {
    deviceExtensions.emplace_back(extension, optional);
}

/*
 *  @begin function to check if the requested layer properties are available or not
 * */
bool Context::checkInstanceEnablesArray(const std::vector<VkLayerProperties>& properties, const ContextCreateInfo::ApiFeatureArray& requestedProperties, std::vector<const char*>& availableProperties) {
    for (const auto& layers : requestedProperties) {
        bool layerFound = false;
        for (const auto& layer : properties) {
            if (strcmp(layers.name.c_str(), layer.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (layerFound) {
            availableProperties.push_back(layers.name.c_str());
        } else if (!layers.optional) {
            LOG("Layer Not Present: %s", layers.name.c_str());
            return false;
        }
    }
    return true;
}

/*
 *  @begin function to check if the requested extension properties are available or not
 * */
bool Context::checkInstanceEnablesArray(const std::vector<VkExtensionProperties>& properties, const ContextCreateInfo::ApiFeatureArray& requestedProperties, std::vector<const char*>& availableProperties) {
    for (const auto& extensions : requestedProperties) {
        bool extensionFound = false;
        for (const auto& extension : properties) {
            if (strcmp(extensions.name.c_str(), extension.extensionName) == 0) {
                extensionFound = true;
            }
        }

        if (extensionFound) {
            availableProperties.push_back(extensions.name.c_str());
        } else if (!extensions.optional) {
            LOG("Extension Not Present: %s", extensions.name.c_str());
            return false;
        }
    }
    return true;
}

/*
 *  @begin this function is a callback function that determines which type of messages should be caught for display.
 * */
VKAPI_ATTR VkBool32 VKAPI_CALL Context::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData
) {
    (void)messageType; (void)pUserData;
    if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
        DEBUGLOG(VERBOSE, "%s\n", pCallbackData->pMessage);
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
        DEBUGLOG(WARNING, "%s\n", pCallbackData->pMessage);
    } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        DEBUGLOG(ERROR, "%s\n", pCallbackData->pMessage);
    }
    return 0u;
}

/*
 *  @begin this function creates debug messenger create info that fills the info like callback function, types and levels
 *  of messages to display and intercept.
 * */
void Context::addDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugInfo) {
    debugInfo = {};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    debugInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    debugInfo.pfnUserCallback = debugCallback;
    debugInfo.pUserData = this;
}

/*
 *  @begin this function returns queue family index that supports all the requested queues
 * */
uint32_t Context::getQueueFamilyIndex(VkQueueFlags queueFlags) {
    if (queueFlags & VK_QUEUE_COMPUTE_BIT == queueFlags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++) {
            if ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
        }
    }

    if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags) {
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++) {
            if ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
                return i;
            }
        }
    }

    for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++) {
        if ((m_queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
            return i;
        }
    }

    throw std::runtime_error("Could not find a matching queue family index");
}

VkCommandBuffer Context::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = pool;
    commandBufferAllocateInfo.level = level;
    commandBufferAllocateInfo.commandBufferCount = 1;
    VkCommandBuffer cmdBuffer;

    VkResult res = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, &cmdBuffer);
    assert(!res);

    if (begin) {
        VkCommandBufferBeginInfo cmdBufferBeginInfo {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);
    }
    return cmdBuffer;
}
        
VkCommandBuffer Context::createCommandBuffer(VkCommandBufferLevel level, bool begin)
{
    return createCommandBuffer(level, m_commandPool, begin);
}

void Context::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free) {
    if (commandBuffer == VK_NULL_HANDLE) {
        return;
    }

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    // Create fence to ensure that the command buffer has finished executing
    VkFenceCreateInfo fenceCreateInfo {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;

    VkFence fence;
    vkCreateFence(m_device, &fenceCreateInfo, nullptr, &fence);

    // Submit to the queue
    vkQueueSubmit(queue, 1, &submitInfo, fence);

    // Wait for the fence to signal that command buffer has finished executing
    vkWaitForFences(m_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT);
    vkDestroyFence(m_device, fence, nullptr);

    if (free) {
        vkFreeCommandBuffers(m_device, pool, 1, &commandBuffer);
    }
}

void Context::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free) {
    return flushCommandBuffer(commandBuffer, queue, m_commandPool, free);
}

VkResult Context::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data)
{
    // Create the buffer handle
    VkBufferCreateInfo bufCreateInfo {};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufCreateInfo.usage = usageFlags;
    bufCreateInfo.size = size;
    bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(m_device, &bufCreateInfo, nullptr, buffer);

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    vkGetBufferMemoryRequirements(m_device, *buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = findMemoryType(m_physicalDevice, memReqs.memoryTypeBits, memoryPropertyFlags);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    vkAllocateMemory(m_device, &memAlloc, nullptr, memory);
        
    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        void *mapped;
        vkMapMemory(m_device, *memory, 0, size, 0, &mapped);
        memcpy(mapped, data, size);
        // If host coherency hasn't been requested, do a manual flush to make writes visible
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            VkMappedMemoryRange mappedRange{
                .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                .memory = *memory,
                .size = size
            };
            vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);
        }
        vkUnmapMemory(m_device, *memory);
    }

    // Attach the memory to the buffer object
    vkBindBufferMemory(m_device, *buffer, *memory, 0);

    return VK_SUCCESS;
}

VkResult Context::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer *buffer, VkDeviceSize size, void *data)
{
    buffer->device = m_device;

    // Create the buffer handle
    VkBufferCreateInfo bufCreateInfo {};
    bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufCreateInfo.usage = usageFlags;
    bufCreateInfo.size = size;
    vkCreateBuffer(m_device, &bufCreateInfo, nullptr, &buffer->buffer);

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    vkGetBufferMemoryRequirements(m_device, buffer->buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = findMemoryType(m_physicalDevice, memReqs.memoryTypeBits, memoryPropertyFlags);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    vkAllocateMemory(m_device, &memAlloc, nullptr, &buffer->bufferMemory);

    buffer->alignment = memReqs.alignment;
    buffer->bufferSize = size;
    buffer->usage = usageFlags;
    buffer->memoryPropertyFlags = memoryPropertyFlags;

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        buffer->map();
        memcpy(buffer->bufferData, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            buffer->flush();

        buffer->unmap();
    }

    // Initialize a default descriptor that covers the whole buffer size
    buffer->setupDescriptor();

    // Attach the memory to the buffer object
    return buffer->bindBuffer();
}

void Context::copyBuffer(Buffer *src, Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion) {
    assert(dst->bufferSize >= src->bufferSize);
    assert(src->buffer);
    VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy bufferCopy{};
    if (copyRegion == nullptr)
    {
        bufferCopy.size = src->bufferSize;
    }
    else
    {
        bufferCopy = *copyRegion;
    }

    vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);

    flushCommandBuffer(copyCmd, queue);
}
}
