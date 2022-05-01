#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <window.h>
#include<app_state.h>

// std
#include <vector>

class vk_engine;
enum class problems : uint8_t;
namespace gui
{
    class interface
    {
        vk_engine *_vk_engine;
        Window &_wnd;
        VkRenderPass _renderpass;

        VkCommandPool _command_pool;

        std::vector<VkCommandBuffer> _command_buffers;

        void create_descriptor_pool();
        VkDescriptorPool _descriptor_pool;

        void upload_fonts();
        void create_renderpass();
        void create_command_buffer();
        void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
        void create_frame_buffers();
        void init_interface();

        std::vector<VkFramebuffer> _frame_buffers;
        VkImageView _attachment;

        ImFont *_font;

        rotation_state *_rot_state;
        void create_gui_layout();

        bool main_window_active;
        bool first_problem;

        problems _current_problem;
        problems _previous_problem;

    public:
        void init_interface_layout(void (*user_layout)());
        interface(Window &wnd, vk_engine *vk_eng);
        void set_rotation_state(rotation_state *);
        const rotation_state &get_rotation_state();
        ~interface();
        void draw(uint32_t);
        VkCommandBuffer get_command_buffer(uint32_t image_index);
    };
}