#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#include "core/swapchain.h"
#include "core/Core.h"

namespace Uriel {

/*
 *  @begin this function initializes the instance, physical device and device object
 * */
void Swapchain::setup(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
    this->instance = instance;
    this->physicalDevice = physicalDevice;
    this->device = device;
}

/*
 *  @begin this function creates surface using glfwCreateWindowSurface function.
 *  Surface creation is an abstraction representing the native window or display area
 *  where rendered images are eventually displayed
 * */
void Swapchain::createSurface(GLFWwindow* window) {
    if (instance == VK_NULL_HANDLE) {
        throw std::runtime_error("Vulkan instance is NULL");
    }

    VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (err != VK_SUCCESS) {
        throw std::runtime_error("glfwCreateWindowSurface failed");
    }

    assert(surface != VK_NULL_HANDLE);

    LOG_INIT("Surface");
}

/*
 *  @begin 
 *  Swapchain is the array of rendered images that is used for rendering and presentation on the surface
 *  Here, we choose the queue family that supports graphics queue and presentation queue (which is surface support)
 *  We also choose the surface format, surface capabilities, extent, present modes, transforms, swapchain images and
 *  use them for creating swapchain create info and then swapchain object
 *  This function also creates image views for all the swapchain images.
 *
 * */
void Swapchain::createSwapchain(uint32_t width, uint32_t height, bool vsync) {
    VkSwapchainKHR oldSwapchain = m_swapchain;

    //  NOTE: finding queue family supporting graphics and present
    uint32_t queueCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueProps(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

    std::vector<VkBool32> presentSupport(queueCount);
    for (uint32_t i = 0; i < queueCount; i++) {
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport[i]);
    }

    //  INFO: Choosing the queue family for graphics and presentation queue
    uint32_t graphicsFamilyIndex = std::numeric_limits<uint32_t>::max();
    uint32_t presentFamilyIndex = std::numeric_limits<uint32_t>::max();

    for (uint32_t i = 0; i < queueCount; i++) {
        if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
            if (graphicsFamilyIndex == std::numeric_limits<uint32_t>::max()) {
                graphicsFamilyIndex = i;
            }
            if (presentSupport[i] == VK_TRUE) {
                graphicsFamilyIndex = i;
                presentFamilyIndex = i;
                break;
            }
        }
    }

    if (graphicsFamilyIndex != presentFamilyIndex) {
        throw std::runtime_error("Graphics and Present Queues both are to be supported!");
    }

    queueFamilyIndex = graphicsFamilyIndex;

    //  INFO: Determining the surface format and capabilities and extent (size of the screen)
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    assert(formatCount > 0);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data());

    VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
    for (auto& availableFormat : surfaceFormats) {
        if (std::find(preferredImageFormats.begin(), preferredImageFormats.end(), availableFormat.format) != preferredImageFormats.end()) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    m_surfaceFormat = surfaceFormat.format;
    m_colorSpace = surfaceFormat.colorSpace;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
        m_extent.width = width;
        m_extent.height = height;
    } else {
        m_extent = surfaceCapabilities.currentExtent;
        width = surfaceCapabilities.currentExtent.width;
        height = surfaceCapabilities.currentExtent.height;
    }

    //  INFO: Choosing present mode for defining how rendered frames are transferred to the screen
    uint32_t presentModeCount{};
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

    //  TODO: Can make it better
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (!vsync) {
		for (size_t i = 0; i < presentModeCount; i++) {
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

    //  INFO: Defining the number of swapchain images that are rendered at a time
    uint32_t desiredSwapchainImages = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && desiredSwapchainImages > surfaceCapabilities.maxImageCount) {
        desiredSwapchainImages = surfaceCapabilities.maxImageCount;
    }

    VkSurfaceTransformFlagBitsKHR transform{};
    if ((surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) != 0u) {
        transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        transform = surfaceCapabilities.currentTransform;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType                  = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface                = surface;
    swapchainCreateInfo.minImageCount          = desiredSwapchainImages;
    swapchainCreateInfo.imageFormat            = m_surfaceFormat;
    swapchainCreateInfo.imageColorSpace        = m_colorSpace;
    swapchainCreateInfo.imageExtent            = m_extent;
    swapchainCreateInfo.imageUsage             = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.preTransform           = transform;
    swapchainCreateInfo.compositeAlpha         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.imageArrayLayers       = 1;
    swapchainCreateInfo.imageSharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount  = 0;
    swapchainCreateInfo.presentMode            = swapchainPresentMode;
    swapchainCreateInfo.oldSwapchain           = oldSwapchain;
    swapchainCreateInfo.clipped                = VK_TRUE;

	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	// Enable transfer destination on swap chain images if supported
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		swapchainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

    VkResult res = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &m_swapchain);
    assert(!res);

    LOG_INIT("Swapchain");

    //  INFO: When new image has to be rendered in place of the old one
	if (oldSwapchain != VK_NULL_HANDLE) { 
		for (auto i = 0; i < m_images.size(); i++) {
			vkDestroyImageView(device, m_imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
	}

    //  INFO: Getting swapchain images
	vkGetSwapchainImagesKHR(device, m_swapchain, &m_imageCount, nullptr);
	m_images.resize(m_imageCount);
	vkGetSwapchainImagesKHR(device, m_swapchain, &m_imageCount, m_images.data());

    m_imageViews.resize(m_imageCount);

    //  INFO: Creating image view for each image
    for (auto i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo colorViewCreateInfo{};
        colorViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorViewCreateInfo.image = m_images[i];
        colorViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorViewCreateInfo.format = m_surfaceFormat;
        colorViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        colorViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorViewCreateInfo.subresourceRange.baseMipLevel = 0;
        colorViewCreateInfo.subresourceRange.levelCount = 1;
        colorViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        colorViewCreateInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(device, &colorViewCreateInfo, nullptr, &m_imageViews[i]);
    }

    LOG_INIT("SwapchainImageView");
}

/*
 *  @begin this function is to get the next swapchain image that will be shown on the screen
 * */
VkResult Swapchain::acquireNextImage(VkSemaphore activeSemaphore, uint32_t& imageIndex) {
    VkResult res = vkAcquireNextImageKHR(device, m_swapchain, UINT64_MAX, activeSemaphore, (VkFence)nullptr, &imageIndex);
    assert(!res);
    return res;
}

/*
 *  @begin this function is used when the old swapchain handle needs to be replaced with the new swachain handle
 * */
void Swapchain::cleanup() {
	if (m_swapchain != VK_NULL_HANDLE) {
		for (auto i = 0; i < m_images.size(); i++) {
			vkDestroyImageView(device, m_imageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(device, m_swapchain, nullptr);
	}
	if (surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
	surface = VK_NULL_HANDLE;
	m_swapchain = VK_NULL_HANDLE;
}

}
