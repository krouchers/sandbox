#pragma once
#include <vulkan/vulkan_core.h>
#include <vulkan_context.h>

class depth_buffer
{
public:
    depth_buffer(vulkan_context *vk_cont);
    ~depth_buffer();

    static bool is_having_stencil_comp(VkFormat);
    VkImageView get_image_view();

private:
    vulkan_context *_vk_context;

    VkImage _depth_image;
    VkImageView _depth_image_view;
    VkDeviceMemory _depth_image_memory;
};