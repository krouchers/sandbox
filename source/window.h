#pragma once
// std
#include <string>
//
// 3dparty
#include <glm/vec2.hpp>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

class Window
{
public:
    GLFWwindow *getglfwWindow();
    Window(int width, int height, std::string name);
    ~Window();

    Window(const Window &) = delete;
    Window(const Window &&) = delete;
    Window &operator=(const Window &) = delete;
    // seters
    void set_user_pointer(void *pointer);

    // geters
    glm::vec2 get_cursor_pos();
    int get_height();
    int get_width();
    //
private:
    void run();
    void Init(int width, int height, std::string name);
    GLFWwindow *_window;
    static void resizeCallBack(GLFWwindow *window, int width, int height);
    static void cursorMovedCallBack(GLFWwindow *window, double widht, double height);
    static void mauseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    int _width, _height;
};