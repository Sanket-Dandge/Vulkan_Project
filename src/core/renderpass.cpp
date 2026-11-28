#include "core/renderpass.h"

namespace capella {
    uint32_t Renderpass::createAttachment(VkFormat format, VkSampleCountFlagBits sampleCount, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, VkImageLayout initialLayout, VkImageLayout finalLayout, VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) {
        if (mRenderpass != VK_NULL_HANDLE) {
            throw std::logic_error("Renderpass already exists. Cannot add new attachments");
        }

        VkAttachmentDescription attachment{};
        attachment.format = format;
        attachment.loadOp = loadOp;
        attachment.storeOp = storeOp;
        attachment.stencilLoadOp = stencilLoadOp;
        attachment.stencilStoreOp = stencilStoreOp;
        attachment.samples = sampleCount;
        attachment.initialLayout = initialLayout;
        attachment.finalLayout= finalLayout;

        attachments.push_back(attachment);
        return static_cast<uint32_t>(attachments.size() - 1);
    }

    bool Renderpass::isSupportedDepthFormat(VkFormat format) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return true;
        }
        return false;
    }

    VkAttachmentReference Renderpass::createAttachmentReference(uint32_t attachmentIndex, VkImageLayout layout) {
        if (attachmentIndex >= attachments.size()) {
            throw std::out_of_range("Invalid Renderpass attachment index");
        }
        VkAttachmentReference attachmentRef{};
        attachmentRef.attachment = attachmentIndex;
        attachmentRef.layout = layout;

        return attachmentRef;
    }

    uint32_t Renderpass::createSubpass(VkPipelineBindPoint pipelineBindPoint, std::vector<VkAttachmentReference>& colorAttachmentRefs, std::optional<VkAttachmentReference> depthAttachmentRef) {
        SubpassInfo subpass{};
        subpass.pipelineBindPoint = pipelineBindPoint;
        subpass.colorAttachments = colorAttachmentRefs;
        subpass.depthAttachment = depthAttachmentRef;

        subpasses.push_back(subpass);
        return static_cast<uint32_t>(subpasses.size() - 1);
    }

    void Renderpass::createSubpassDependency(const VkSubpassDependency& dependency) {
        subpassDependencies.push_back(dependency);
    }

    void Renderpass::createRenderpass() {
        std::vector<VkSubpassDescription> subpassDescriptions(subpasses.size());

        for (int i = 0; i < subpasses.size(); i++) {
            const SubpassInfo& subpass = subpasses[i];
            VkSubpassDescription subDesc = subpassDescriptions[i];
            subDesc.pipelineBindPoint = subpass.pipelineBindPoint;
            subDesc.colorAttachmentCount = static_cast<uint32_t>(subpass.colorAttachments.size());
            subDesc.pColorAttachments = subpass.colorAttachments.data();
            subDesc.pDepthStencilAttachment = &subpass.depthAttachment.value();
        }

        VkRenderPassCreateInfo renderpassInfo{};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
        renderpassInfo.attachmentCount = attachments.size();
        renderpassInfo.pAttachments = attachments.data();
        renderpassInfo.subpassCount = subpasses.size();
        renderpassInfo.pSubpasses = subpassDescriptions.data();
        renderpassInfo.dependencyCount = subpassDependencies.size();
        renderpassInfo.pDependencies = subpassDependencies.data();

        vkCreateRenderPass(device, &renderpassInfo, nullptr, &mRenderpass);
    }

    void Renderpass::destroy() {
        vkDestroyRenderPass(device, mRenderpass, nullptr);
        mRenderpass = nullptr;
    }

}
