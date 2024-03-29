#include <vk_engine.h>
#include <mesh.h>
#include <texture.h>
#include <application.h>

vk_engine::vk_engine(Window &window, application &app) : wnd{window}, _app{app}
{
    init_engine();
}

vk_engine::~vk_engine()
{
    context->device_idle();
    context->destroy_final_vertex_buffer();
}

void vk_engine::init_engine()
{
#ifdef DEBUG
    context = std::make_unique<vulkan_context>(wnd, true);
#else
    context = std::make_unique<vulkan_context>(wnd);
#endif
    mesh my_mesh{"../assets/models/cube_with_cells.obj", "../assets/textures/cells.png"};
    load_mesh(my_mesh);
    _app.get_app_state().current_problem = problems_list::CUBE;
}

void vk_engine::load_mesh(mesh &arg)
{
    context->load_mesh(arg);
    context->get_swapchain().record_all_command_buffers();
}

void vk_engine::draw_frame()
{
    context->draw_frame();
}

vulkan_context *vk_engine::get_vk_context()
{
    return context.get();
}

void vk_engine::normalize_coordinats(glm::vec2 &pos_to_normalize)
{
    pos_to_normalize.x = (pos_to_normalize.x / wnd.get_width() * 2) - 1;
    pos_to_normalize.y = (pos_to_normalize.y / wnd.get_height() * 2) - 1;
}

void vk_engine::destroy_loaded_mesh()
{
    vkDeviceWaitIdle(context->get_logical_device().get_vk_handler());
    context->destroy_final_vertex_buffer();
    context->destroy_final_index_buffer();
    context->destroy_descriptor_sets();
}

void vk_engine::recreate_swapchain()
{
    context->recreate_swapchain();
}