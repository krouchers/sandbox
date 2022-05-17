#pragma once
#include <window.h>
#include <vk_engine.h>
#include <imgui/interface.h>
#include "IO.h"
#include<problems.h>    

// std
#include <memory>

class application
{
    std::unique_ptr<vk_engine> my_engine;
    std::unique_ptr<Window> window;
    std::unique_ptr<gui::interface> _interface;

    app_state _app_state;

    application_file_system _file_system;
    std::unique_ptr<problems> _problems;

public:
    application(int width, int height, std::string app_name, char *app_path);
    //geters
    Window *get_window();
    app_state &get_app_state();
    application_file_system &get_file_system();
    problems &get_problems();
    //
    void run();
    void close();
    void draw_frame();
};