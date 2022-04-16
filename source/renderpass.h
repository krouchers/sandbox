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
    void create_framebuffers();
    void create_renderpass();
    VkRenderPass &get_vk_handle();
    std::vector<VkFramebuffer> get_framebuffers();
    renderpass(vulkan_context *);
    ~renderpass();
};