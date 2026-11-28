#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace capella {

class Swapchain {
    public:
        VkFormat m_surfaceFormat{};
        VkColorSpaceKHR m_colorSpace{};
        VkSwapchainKHR m_swapchain{};

        uint32_t queueFamilyIndex = UINT32_MAX;
        uint32_t m_imageCount{0};
        VkExtent2D m_extent{};

        std::vector<VkImage> m_images{};
        std::vector<VkImageView> m_imageViews{};

        void setup(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
        void createSurface(GLFWwindow* window);
        void createSwapchain(uint32_t width, uint32_t height, bool vsync);
        VkResult acquireNextImage(VkSemaphore activeSemaphore, uint32_t& imageIndex);
        void cleanup();

        std::vector<VkFormat> preferredImageFormats = { 
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_FORMAT_R8G8B8A8_UNORM, 
            VK_FORMAT_A8B8G8R8_UNORM_PACK32 
        };


    private:
        VkInstance instance{VK_NULL_HANDLE};
        VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
        VkDevice device{VK_NULL_HANDLE};
        VkSurfaceKHR surface{VK_NULL_HANDLE};

        const char* TAG = "Swapchain";
        
};

}
