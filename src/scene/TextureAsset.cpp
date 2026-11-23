#include "scene/TextureAsset.h"
#include "core/context.h"
#include "utils.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vulkan/vulkan_core.h>

namespace capella {

void TextureAsset::loadglTFImage(tinygltf::Image& gltfImage, std::string& filepath, Context* ctx, VkQueue cmdQueue) {
    //  @info: getting gltfImage data in a buffer
    VkDeviceSize gltfImageBufSize = gltfImage.width * gltfImage.height * 4;
    unsigned char* gltfImageBuf = new unsigned char[gltfImageBufSize];
    if (gltfImage.component == 3) {
        for (int i = 0; i < gltfImage.width * gltfImage.height; i++) {
            unsigned char* rgb = &gltfImage.image[0];
            for (int j = 0; j < gltfImage.component; j++) {
                gltfImageBuf[j] = rgb[j];
                gltfImageBuf += 4;
                rgb += 3;
            }
        }
    } else {
        gltfImageBuf = &gltfImage.image[0];
        gltfImageBufSize = gltfImage.image.size();
    }
    
    //  @info: creating gltfImage buffer
    VkBuffer stagingbuffer;
    VkDeviceMemory stagingBufferMemory;

    width = gltfImage.width;
    height = gltfImage.height;

    ctx->createBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, gltfImageBufSize, &stagingbuffer, &stagingBufferMemory, gltfImageBuf);

    //  @info: create gltfImage
    VkImageCreateInfo gltfImageInfo{};
    gltfImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    gltfImageInfo.imageType = VK_IMAGE_TYPE_2D;
    gltfImageInfo.extent.width = gltfImage.width;
    gltfImageInfo.extent.height = gltfImage.height;
    gltfImageInfo.extent.depth = 1;
    gltfImageInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(gltfImage.width, gltfImage.height))) + 1.0);
    gltfImageInfo.arrayLayers = 1;
    gltfImageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    gltfImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    gltfImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    gltfImageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    gltfImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    gltfImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateImage(ctx->m_device, &gltfImageInfo, nullptr, &image);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(ctx->m_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(ctx->m_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(ctx->m_device, &allocInfo, nullptr, &imagememory);
    vkBindImageMemory(ctx->m_device, image, imagememory, 0);

    //  @info: transitioning image layout so that pipeline can safely use it
    VkCommandBuffer cmdBuffer = ctx->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkImageMemoryBarrier barrier1{};
    barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier1.image = image;
    barrier1.srcAccessMask = 0;
    barrier1.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier1.subresourceRange.levelCount = 1;
    barrier1.subresourceRange.baseArrayLayer = 0;
    barrier1.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);

    VkBufferImageCopy imgBufCopy{};
    imgBufCopy.bufferOffset = 0;
    imgBufCopy.bufferRowLength = 0;
    imgBufCopy.bufferImageHeight = 0;
    imgBufCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imgBufCopy.imageSubresource.mipLevel = 0;
    imgBufCopy.imageSubresource.baseArrayLayer = 0;
    imgBufCopy.imageSubresource.layerCount = 1;
    imgBufCopy.imageOffset = {0, 0, 0};
    imgBufCopy.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(cmdBuffer, stagingbuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgBufCopy);

    VkImageMemoryBarrier barrier2{};
    barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier2.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier2.image = image;
    barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier2.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier2.subresourceRange.levelCount = 1;
    barrier2.subresourceRange.baseArrayLayer = 0;
    barrier2.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);
    ctx->flushCommandBuffer(cmdBuffer, cmdQueue, true);

    vkDestroyBuffer(ctx->m_device, stagingbuffer, nullptr);
    vkFreeMemory(ctx->m_device, stagingBufferMemory, nullptr);

    //  @info: Generating Mipmaps for more detailing
    generateMipMaps(ctx, cmdQueue);

    //  @info: Creating image sampler
    createSamplers(ctx);
}

void TextureAsset::generateMipMaps(Context* ctx, VkQueue blitQueue) {
    VkCommandBuffer blitCmd = ctx->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkImageMemoryBarrier barrier1{};
    barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier1.image = image;
    barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier1.subresourceRange.baseArrayLayer = 0;
    barrier1.subresourceRange.layerCount = 1;
    barrier1.subresourceRange.levelCount = 1;

    for (uint32_t i = 1; i < miplevels; i++) {
        barrier1.subresourceRange.baseMipLevel = i - 1;
        barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier1.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier1.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);

        VkImageBlit imageBlit{};
        imageBlit.srcOffsets[0] = { 0, 0, 0 };
        imageBlit.srcOffsets[1] = { int32_t(width >> (i - 1)), int32_t(height >> (i - 1)), 1};

        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.mipLevel = i - 1;
        imageBlit.srcSubresource.baseArrayLayer = 0;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.dstOffsets[0] = {0, 0, 0};
        imageBlit.dstOffsets[1] = { int32_t(width >> i), int32_t(height >> i), 1 };
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.mipLevel = i;
        imageBlit.dstSubresource.baseArrayLayer = 0;
        imageBlit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(blitCmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

        barrier1.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier1.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier1.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);
    }

    barrier1.subresourceRange.baseMipLevel = miplevels - 1;
    barrier1.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier1.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier1.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier1.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);
    ctx->flushCommandBuffer(blitCmd, blitQueue, true);
}

void TextureAsset::createSamplers(Context* ctx) {
    VkImageViewCreateInfo imageViewInfo{};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.image = image;
    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewInfo.subresourceRange.levelCount = miplevels;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;

    vkCreateImageView(ctx->m_device, &imageViewInfo, nullptr, &imageview);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 0.f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.maxLod = miplevels;
    samplerInfo.mipLodBias = 0.0f;

    vkCreateSampler(ctx->m_device, &samplerInfo, nullptr, &imagesampler);
}

}
