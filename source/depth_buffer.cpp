#include <depth_buffer.h>
#include <stdexcept>
#include <swapchain.h>

depth_buffer::depth_buffer(vulkan_context *vk_cont) : _vk_context{vk_cont}
{
    _vk_context->create_image(
        _vk_context->get_swapchain().get_extent().width,
        _vk_context->get_swapchain().get_extent().height,
        _vk_context->find_depth_format(), VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depth_image, _depth_image_memory);

    _depth_image_view = _vk_context->create_image_view(_depth_image, _vk_context->find_depth_format(), VK_IMAGE_ASPECT_DEPTH_BIT);
    _vk_context->transition_image_layout(
        _depth_image,
        _vk_context->find_depth_format(),
        VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}


VkImageView depth_buffer::get_image_view()
{
    return _depth_image_view;
}
depth_buffer::~depth_buffer()
{
    vkDestroyImage(_vk_context->get_logical_device().get_vk_handler(), _depth_image, nullptr);
    vkDestroyImageView(_vk_context->get_logical_device().get_vk_handler(), _depth_image_view, nullptr);
    vkFreeMemory(_vk_context->get_logical_device().get_vk_handler(), _depth_image_memory, nullptr);
};