#pragma once
#include<vulkan/vulkan.h>
#include<depth_buffer.h>
#include<memory>

// std
#include<vector>

class renderpass
{

    vulkan_context *_vk_context;
    VkRenderPass _renderpass;

    std::unique_ptr<depth_buffer> _depth_buffer;

    std::vector<VkFramebuffer> swapchain_framebuffers;

public:
    renderpass(vulkan_context *);
    ~renderpass();
    void create_framebuffers();
    void create_renderpass();
    void recreate_framebuffers();
    void destroy_framebuffers();
    VkRenderPass &get_vk_handle();
    std::vector<VkFramebuffer> &get_framebuffers();
    //geters
    void destroy_depth_buffer();
    void create_depth_buffer();
};