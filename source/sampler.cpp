#include <sampler.h>
#include <stdexcept>

sampler::~sampler()
{
    vkDestroySampler(_vk_context->get_logical_device().get_vk_handler(), _sampler, nullptr);
}
sampler::sampler(vulkan_context *vk_cont) : _vk_context{vk_cont}
{
    VkPhysicalDeviceProperties device_property{};
    _vk_context->get_physical_device().getDeviceProperties(_vk_context->get_physical_device().getVkHandler(), &device_property);
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipLodBias = 0;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = device_property.limits.maxSamplerAnisotropy;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.minLod = 0;
    sampler_info.maxLod = 0;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    if (vkCreateSampler(_vk_context->get_logical_device().get_vk_handler(),
                        &sampler_info, nullptr, &_sampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create sampler");
}

VkSampler &sampler::get_vk_handle(){
    return _sampler;
}