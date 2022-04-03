#pragma once
#include <vulkan_context.h>
#include<window.h>

//std
#include<memory>

class vk_engine
{
    void init_engine();
    std::unique_ptr<vulkan_context> context;

    Window &wnd;

public:
    vk_engine(Window &wnd);
    ~vk_engine();

    void normalize_coordinats(glm::vec2 &);
    void create_custom_size_buffer(size_t size);
    void load_obj(std::vector<Vertex> &&obj);
    void draw_frame();
};