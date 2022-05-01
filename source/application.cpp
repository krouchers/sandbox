#include <application.h>
#include <vk_engine.h>
#include <primitives.h>
// std
#include <vector>
//

application::application(int width, int height, std::string app_name) : _app_state{}
{
    window = std::make_unique<Window>(width, height, app_name);
    window->set_user_pointer(this);
    my_engine = std::make_unique<vk_engine>(*window.get());
    _interface = std::make_unique<gui::interface>(*window.get(), my_engine.get());
    my_engine->get_vk_context()->init_interface(_interface.get(), &_app_state.rot_state);
}

void application::run()
{
    while (!glfwWindowShouldClose(window->getglfwWindow()))
    {
        glfwPollEvents();
        draw_frame();
    }
}

void application::draw_frame()
{
    my_engine->draw_frame();
}

Window *application::get_window()
{
    return window.get();
}