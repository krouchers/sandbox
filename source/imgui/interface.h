#pragma once
#include <imgui.h>
#include<imgui_impl_glfw.h>
#include <vulkan_context.h>
#include<window.h>

class interface
{
    vulkan_context *_vk_context;
    Window &_wnd;

    void create_descriptor_pool();
    VkDescriptorPool _descriptor_pool;

public:
    interface(Window &);
    void init_interface(vulkan_context *vk_context);
    ~interface();
};