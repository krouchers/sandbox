#include <application.h>
#include <vk_engine.h>
#include <primitives.h>
#include <problems.h>
// std
#include <vector>
//

application::application(int width, int height, std::string app_name, char *app_path) : _app_state{}, _file_system{app_path}
{
    window = std::make_unique<Window>(width, height, app_name);
    window->set_user_pointer(this);
    _problems = std::make_unique<problems>(this);
    my_engine = std::make_unique<vk_engine>(*window.get(), *this);
    _interface = std::make_unique<gui::interface>(*window.get(), my_engine.get(), *this);
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

app_state &application::get_app_state()
{
    return _app_state;
}
application_file_system &application::get_file_system()
{
    return _file_system;
}

problems &application::get_problems()
{
    return *_problems.get();
}
void application::close()
{
    glfwSetWindowShouldClose(window->getglfwWindow(), true);
}