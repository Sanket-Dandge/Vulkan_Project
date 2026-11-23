#pragma once

#include "buffers.h"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace capella {

struct ContextCreateInfo {
    //  @info: Vulkan Application Info
    const char* appName = "GameScene";
    const char* engineName = "Capella";

    uint32_t apiMajor = 1;
    uint32_t apiMinor = 3;
    uint32_t apiPatch = 0;

    //  @info: logs enabler
    bool enableValidationLayer = false;

    //  @info: extension/layer info
    struct ApiFeature {
        ApiFeature(const char* name, bool isOptional = false)
            :   name(name), optional(isOptional) {}
        std::string name;
        bool optional{false};
    };

    using ApiFeatureArray = std::vector<ApiFeature>;
    ApiFeatureArray instanceLayers;
    ApiFeatureArray instanceExtensions;
    ApiFeatureArray deviceExtensions;

    //  @info: info on which queue are available in a queuefamily
    struct QueueFamilyIndices {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    };

    QueueFamilyIndices queueFamilyIndices;
    
    std::vector<const char*> m_availableInstanceLayers;
    std::vector<const char*> m_availableInstanceExtensions;
    std::vector<const char*> m_availableDeviceExtensions;

    VkQueueFlags requestedQueues;

    void addInstanceLayer(const char* layerName, bool optional = false);
    void addInstanceExtension(const char* extensionName, bool optional = false);
    void setVersion(uint32_t major, uint32_t minor);
    void addDeviceExtension(const char* deviceExtension, bool optional = false);

    void removeInstanceLayer(const char* name);
    void removeInstanceExtension(const char* name);
};

class Context {
    public:
        void initializeInstance(ContextCreateInfo& info);
        void createLogicalDevice(ContextCreateInfo& info, VkPhysicalDeviceFeatures requiredFeatures);
        void pickPhysicalDevice(ContextCreateInfo& info, uint32_t index);
        void enableDebugging();
        VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = 0);
        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
        VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
        void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
        void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);

        VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr);
        VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer *buffer, VkDeviceSize size, void *data = nullptr);
        void copyBuffer(Buffer *src, Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion = nullptr);

        bool checkInstanceEnablesArray(const std::vector<VkLayerProperties>& properties, const ContextCreateInfo::ApiFeatureArray& requestedProperties, std::vector<const char*>& availableProperties);
        bool checkInstanceEnablesArray(const std::vector<VkExtensionProperties>& properties, const ContextCreateInfo::ApiFeatureArray& requestedProperties, std::vector<const char*>& availableProperties);

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData
        );
        void addDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugInfo);

        std::vector<VkLayerProperties> getInstanceLayers();
        std::vector<VkExtensionProperties> getInstanceExtensions();
        std::vector<VkExtensionProperties> getDeviceExtensions(VkPhysicalDevice physicalDevice);
        uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags);

        VkInstance m_instance{VK_NULL_HANDLE};
        PFN_vkCreateDebugUtilsMessengerEXT m_createDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT m_destroyDebugUtilsMessengerEXT = nullptr;
        VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
        VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
        VkDevice m_device{VK_NULL_HANDLE};
        VkQueue m_graphicsQueue{VK_NULL_HANDLE};
        VkQueue m_computeQueue{VK_NULL_HANDLE};
        VkQueue m_transferQueue{VK_NULL_HANDLE};
        VkCommandPool m_commandPool{ VK_NULL_HANDLE };;

        std::vector<VkQueueFamilyProperties> m_queueFamilyProperties{};
        VkPhysicalDeviceMemoryProperties m_memoryProperties{};
        VkPhysicalDeviceFeatures deviceFeatures{};
        VkPhysicalDeviceFeatures requiredFeatures{};
        VkPhysicalDeviceProperties m_properties{};
        uint32_t queueIndex{};
};

}
