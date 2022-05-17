#pragma once
#include <vulkan_context.h>
#include <window.h>
#include <interface.h>

// std
#include <memory>

class vk_engine
{
    void init_engine();
    std::unique_ptr<vulkan_context> context;

    Window &wnd;
    application &_app;

public:
    vk_engine(Window &wnd, application &app);
    ~vk_engine();

    vulkan_context *get_vk_context();
    void load_texture(const std::string texture_path);
    void load_mesh(mesh &);
    void destroy_loaded_mesh();
    void normalize_coordinats(glm::vec2 &);
    void create_custom_size_buffer(size_t size);
    void draw_frame();
};