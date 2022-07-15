#pragma once
// std
#include <string>
//
// 3dparty
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

struct pos
{
    int x, y;
};

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

    glm::vec3 mouse_pos{0, 0, 0};
    glm::vec3 delta{0, 0, 0};
    float scroll{-2.5f};
    glm::vec2 translation{0, 0};
    // FIXME
    bool prev_event = false;
    bool current_event = false;
    //
    // geters
    glm::vec2 get_cursor_pos();
    uint32_t get_height();
    uint32_t get_width();
    //
private:
    void run();
    void Init(int width, int height, std::string name);
    GLFWwindow *_window;
    static void resizeCallBack(GLFWwindow *window, int width, int height);
    static void cursorMovedCallBack(GLFWwindow *window, double widht, double height);
    static void mauseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
    uint32_t _width, _height;
};