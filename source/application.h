#pragma once
#include <window.h>
#include <vk_engine.h>
#include<imgui/interface.h>

// std
#include <memory>

class application
{
    std::unique_ptr<vk_engine> my_engine;
    std::unique_ptr<Window> window;
    std::unique_ptr<gui::interface> _interface;

public:
    application(int width, int height, std::string app_name);
    Window *get_window();
    void load_object(std::vector<Vertex> &&obj);
    void draw_triangle(glm::vec2);
    glm::vec2 get_mause_position();
    void run();
    void draw_frame();
};