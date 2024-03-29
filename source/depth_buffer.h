#pragma once
#include <vulkan/vulkan_core.h>

class vulkan_context;
class depth_buffer
{
public:
    depth_buffer(vulkan_context *vk_cont);
    ~depth_buffer();

    void destroy_image();
    void create_image();
    VkImageView get_image_view();

private:
    vulkan_context *_vk_context;

    VkImage _depth_image;
    VkImageView _depth_image_view;
    VkDeviceMemory _depth_image_memory;
};