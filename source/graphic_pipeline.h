#pragma once
#include <vulkan_context.h>

class graphic_pipeline
{
    VkPipeline _pipeline;
    vulkan_context &_vk_context;
    VkPipelineLayout _pipeline_layout;
    VkShaderModule create_shader_module(std::vector<char> shader_binary_file);

public:
    graphic_pipeline(const graphic_pipeline &) = delete;
    graphic_pipeline(const graphic_pipeline &&) = delete;
    graphic_pipeline &operator=(const graphic_pipeline &) = delete;

    graphic_pipeline(vulkan_context &);
    ~graphic_pipeline();

    void create_graphic_pipeline();
    VkPipeline get_vk_handle();
    std::vector<char> read_shader_file(const char *file_name);
};