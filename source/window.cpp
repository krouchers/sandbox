#include <window.h>
#include <application.h>
// 3dparty
#include <glm/vec2.hpp>

// std
#include <string>
#include <iostream>

Window::Window(int w, int h, std::string name)
    : _width(w), _height(h)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    _window = glfwCreateWindow(_width, _height, name.c_str(), nullptr, nullptr);
    glfwSetFramebufferSizeCallback(_window, resizeCallBack);
    glfwSetCursorPosCallback(_window, cursorMovedCallBack);
    glfwSetWindowUserPointer(_window, this);
    // glfwetMouseButtonCallback(_window, mauseButtonCallback);
}

void Window::run()
{
    while (!glfwWindowShouldClose(_window))
    {
        glfwPollEvents();
    }
}

Window::~Window()
{
    glfwDestroyWindow(_window);
    glfwTerminate();
}

GLFWwindow *Window::getglfwWindow()
{
    return _window;
}

void Window::resizeCallBack(GLFWwindow *window, int width, int height)
{
    std::cout << "window is resized with " << width << " and " << height << std::endl;
    application *app = reinterpret_cast<application *>(glfwGetWindowUserPointer(window));
    app->get_window()->_height = height;
    app->get_window()->_width = width;
    app->get_vk_engine().recreate_swapchain();
    (void)window;
}

void Window::cursorMovedCallBack(GLFWwindow *window, double width, double height)
{
    std::cout << "cursor is resized with " << width << " and " << height << std::endl;
    (void)window;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
    }
}

glm::vec2 Window::get_cursor_pos()
{
    double x, y;
    glfwGetCursorPos(_window, &x, &y);
    glm::vec2 pos{x, y};
    return pos;
}

uint32_t Window::get_height()
{
    return _height;
}

uint32_t Window::get_width()
{
    return _width;
}

void Window::mauseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    (void)window;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        std::cout << "[MOUSE EVENT] button was pressed" << std::endl;
    }
    (void)mods;
}

void Window::set_user_pointer(void *pointer)
{
    glfwSetWindowUserPointer(_window, pointer);
}
