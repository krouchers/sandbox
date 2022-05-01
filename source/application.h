#pragma once
#include <window.h>
#include <vk_engine.h>
#include <imgui/interface.h>
#include "IO.h"
#include<app_state.h>

// std
#include <memory>

enum class problems : uint8_t
{
    CUBE = 0,
    PARAL,
};

class application
{
    std::unique_ptr<vk_engine> my_engine;
    std::unique_ptr<Window> window;
    std::unique_ptr<gui::interface> _interface;

    static void gui_layout();

    app_state _app_state;

public:
    application(int width, int height, std::string app_name);
    Window *get_window();
    void run();
    void draw_frame();
};