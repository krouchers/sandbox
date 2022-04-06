#include <vk_engine.h>

vk_engine::vk_engine(Window &window) : wnd{window}
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
    std::vector<Vertex> data = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
    context->create_vertex_buffer(data);

    std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0};
    context->create_index_buffer(indices);
}

void vk_engine::draw_frame()
{
    context->draw_frame();
}

void vk_engine::normalize_coordinats(glm::vec2 &pos_to_normalize)
{
    pos_to_normalize.x = (pos_to_normalize.x / wnd.get_width() * 2) - 1;
    pos_to_normalize.y = (pos_to_normalize.y / wnd.get_height() * 2) - 1;
}
