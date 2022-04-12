#pragma once
#include<vulkan/vulkan.h>

// std
#include<vector>

class vulkan_context;
class renderpass
{

    vulkan_context &_vk_context;
    VkRenderPass _renderpass;

    std::vector<VkFramebuffer> swapchain_framebuffers;

public:
    void create_framebuffers();
    void create_renderpass();
    VkRenderPass &get_vk_handle();
    std::vector<VkFramebuffer> get_framebuffers();
    renderpass(vulkan_context &);
    ~renderpass();
};