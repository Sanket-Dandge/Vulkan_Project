#include "renderer/textures.h"
#include <ktx.h>

namespace capella {

void Texture::setup(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
    this->instance = instance;
    this->physicalDevice = physicalDevice;
    this->device = device;
}

ktxResult Texture::loadKTXFile(std::string filename, ktxTexture** target) {
    ktxResult result = KTX_SUCCESS;

    result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);

    return result;
}

}
