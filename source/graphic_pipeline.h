#pragma once
#include <vulkan_context.h>
#include <glm/mat4x4.hpp>

class graphic_pipeline
{
    VkPipeline _pipeline;
    vulkan_context &_vk_context;
    VkPipelineLayout _pipeline_layout;
    VkDescriptorSetLayout _descriptor_layout;
    VkShaderModule create_shader_module(std::vector<char> shader_binary_file);

public:
    graphic_pipeline(const graphic_pipeline &) = delete;
    graphic_pipeline(const graphic_pipeline &&) = delete;
    graphic_pipeline &operator=(const graphic_pipeline &) = delete;

    graphic_pipeline(vulkan_context &);
    ~graphic_pipeline();

    void create_graphic_pipeline();
    void create_descriptors();
    // geters
    VkDescriptorSetLayout &get_descriptor_set_layout();
    VkPipeline get_vk_handle();
    std::vector<uniform_buffer_object> &get_ubos();
    VkPipelineLayout &get_pipeline_layout();
    //
    std::vector<char> read_shader_file(const char *file_name);
};