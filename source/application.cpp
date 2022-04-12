#include <application.h>
#include <vk_engine.h>
#include <primitives.h>
// std
#include <vector>
//

application::application(int width, int height, std::string app_name)
{
    window = std::make_unique<Window>(width, height, app_name);
    window->set_user_pointer(this);
    my_engine = std::make_unique<vk_engine>(*window.get());
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

glm::vec2 application::get_mause_position()
{
    return window->get_cursor_pos();
}

void application::draw_triangle(glm::vec2 pos)
{
    uint16_t size_factor{3};
    glm::vec2 left_pos{pos.x - size_factor, pos.y + size_factor};
    glm::vec2 right_pos{(pos.x - left_pos.x) * size_factor + left_pos.x, pos.y + size_factor};
    glm::vec2 up_pos{pos.x, pos.y - size_factor};


}

Window *application::get_window()
{
    return window.get();
}