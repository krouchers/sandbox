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
    context->create_final_vertex_buffer(500 * sizeof(Vertex));
}

void vk_engine::draw_frame(){
    context->draw_frame();
}


void vk_engine::load_obj(std::vector<Vertex> &&obj){
    for(auto &vertex : obj){
        normalize_coordinats(vertex.position);
    }

    context->add_vertex_data(std::move(obj));
}

void vk_engine::normalize_coordinats(glm::vec2 &pos_to_normalize){
    pos_to_normalize.x = (pos_to_normalize.x / wnd.get_width() * 2) - 1;
    pos_to_normalize.y = (pos_to_normalize.y / wnd.get_height() * 2) - 1;
}


void vk_engine::create_custom_size_buffer(size_t quanitiy_of_vertices){
    context->create_staged_vertex_buffer(quanitiy_of_vertices);
}