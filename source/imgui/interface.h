#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <window.h>

// std
#include <vector>

class vulkan_context;

namespace gui
{
    class interface
    {
        vulkan_context *_vk_context;
        Window &_wnd;
        VkRenderPass _renderpass;

        VkCommandPool _command_pool;

        std::vector<VkCommandBuffer> _command_buffers;

        void create_descriptor_pool();
        VkDescriptorPool _descriptor_pool;

        void create_gui_layout();
        void upload_fonts();
        void create_renderpass();
        void create_command_buffer();
        void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
        void create_frame_buffers();

        std::vector<VkFramebuffer> _frame_buffers;
        VkImageView _attachment;

        ImFont *_font;

    public:
        interface(Window &, vulkan_context *);
        void init_interface();
        ~interface();
        void draw(uint32_t);
        VkCommandBuffer get_command_buffer(uint32_t image_index);
    };
}