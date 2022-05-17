#pragma once
#include <stb_image.h>
#include <vulkan/vulkan_core.h>
#include <buffer.h>
#include <vulkan_context.h>
#include <sampler.h>

class texture
{
public:
    texture(vulkan_context *vk_cont, sampler *samp, const std::string tex_path);
    ~texture();

    VkImageView &get_image_view();

    void set_data();

private:
    vulkan_context *_vk_context;
    int _texture_width;
    int _texture_height;
    int _texture_channels;

    const std::string _tex_path;

    VkImage _texture_image;
    VkDeviceMemory _texture_image_memory;

    VkImageView _texture_image_view;

    sampler *_sampler;

    std::unique_ptr<buffer<stbi_uc>> _staging_buffer;
    std::unique_ptr<buffer<stbi_uc>> _local_buffer;

    VkCommandBuffer begin_single_time_commands();
    void create_image_view();
    void create_sampler();
    void end_single_time_commands(VkCommandBuffer command_buffer);
    void set_image_layout(VkImageLayout old_layout, VkImageLayout new_layout);
    void copy_buffer_to_image();
    void transit_from_buffer_to_image();
    void allocate_image_memory();
    void load_image(const std::string path);
    void create_image();
};
