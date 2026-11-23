#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include "helpers/logger.h"

namespace capella {
    struct SubpassInfo {
        VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        std::vector<VkAttachmentReference> colorAttachments;
        std::optional<VkAttachmentReference> depthAttachment;
    };

    class Renderpass {
        public:
            void createRenderpass();

            uint32_t createSubpass(VkPipelineBindPoint pipelineBindPoint,
                               std::vector<VkAttachmentReference>& colorAttachmentRef,
                               std::optional<VkAttachmentReference> depthAttachment = std::nullopt);

            void createSubpassDependency(const VkSubpassDependency& dependency);

            uint32_t createAttachment(
                VkFormat format, VkSampleCountFlagBits sampleCount,
                VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                VkImageLayout initialLayout, VkImageLayout finalLayout,
                VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE);

            VkAttachmentReference createAttachmentReference(uint32_t attachmentIndex, VkImageLayout layout);

            std::vector<VkAttachmentDescription> attachments;
            std::vector<SubpassInfo> subpasses;
            std::vector<VkSubpassDependency> subpassDependencies;

            bool isSupportedDepthFormat(VkFormat format);
            VkRenderPass getRenderpass() { return mRenderpass; }

            void destroy();

          private:
            std::string TAG = "Renderpass";
            VkRenderPass mRenderpass = VK_NULL_HANDLE;

            VkPhysicalDevice physicalDevice;
            VkDevice device;
    };
}

#endif
