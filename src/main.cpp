// #include "core/logger.h"
// #include "scene/SceneLoader.h"
// #include <iostream>
//
// int main() {
//     std::string TAG = "MAIN";
//     LOG_INFO(TAG, "Best day");
//
//     std::string modelFilePath = "assets/models/venus.gltf";
//     capella::SceneLoader loader;
//
//     loader.loadFromFile(modelFilePath);
// };
//

#include "helpers/logger.h"
#include "core/context.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <vulkan/vulkan_core.h>

// GLFW Callback functions
static void onErrorCallback(int error, const char *description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

const int SAMPLE_WIDTH = 1920;
const int SAMPLE_HEIGHT = 1080;


int main (int argc, char *argv[]) {
    (void)argc;

    std::string TAG = "MAIN";
    LOG_INFO(TAG, "Best day");

    glfwSetErrorCallback(onErrorCallback);

    if (!glfwInit()) { return 1; }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(SAMPLE_WIDTH, SAMPLE_HEIGHT, "Base", nullptr, nullptr);
    glfwSetKeyCallback(window, onKeyCallback);

    if (glfwVulkanSupported() == 0) {
        printf("GLFW: Vulkan Not Supported\n");
        return 1;
    }

    uint32_t count{0};
    auto *reqExtensions = glfwGetRequiredInstanceExtensions(&count);

    //-------------------------------ContextCreateInfo-----------------------------------------//
    capella::ContextCreateInfo contextInfo{};
    contextInfo.setVersion(1, 2);

    //  INFO: Adding required extensions (surface, win32, linux, ..)
    for (uint32_t ext_id = 0; ext_id < count; ext_id++) { 
        contextInfo.addInstanceExtension(reqExtensions[ext_id]);
    }

    contextInfo.addInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, true); // Allow debug names
    contextInfo.addDeviceExtension( VK_KHR_SWAPCHAIN_EXTENSION_NAME); // Enabling ability to present rendering
    contextInfo.enableValidationLayer = true;
    contextInfo.requestedQueues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

    // INFO: Creating base context object
    capella::Context ctx{};
    ctx.initializeInstance(contextInfo);

    ctx.pickPhysicalDevice(contextInfo, 0);

    if (ctx.deviceFeatures.samplerAnisotropy) {
        ctx.requiredFeatures.samplerAnisotropy = VK_TRUE;
    }

    ctx.createLogicalDevice(contextInfo, ctx.requiredFeatures);

    while (glfwWindowShouldClose(window) == 0) {
        glfwPollEvents();
        // app.render();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
