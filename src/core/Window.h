#pragma once
#include "Types.h"
#include "Math.h"
#include "Event.h"
#include <string>

struct GLFWwindow;

class Window {
public:
    Window() = default;
    ~Window();

    bool create(i32 width, i32 height, const std::string& title);
    void destroy();
    void pollEvents();
    void swapBuffers();
    bool shouldClose() const;
    void close();

    i32 width() const { return m_width; }
    i32 height() const { return m_height; }
    f32 dpiScale() const { return m_dpiScale; }
    Vec2 size() const { return {(f32)m_width, (f32)m_height}; }

    GLFWwindow* handle() { return m_handle; }
    const GLFWwindow* handle() const { return m_handle; }

    EventDispatcher& events() { return m_events; }
    bool vsync() const { return m_vsync; }
    void setVsync(bool enabled);

    void setTitle(const std::string& title);
    Vec2 mousePos() const;
    bool isKeyDown(i32 key) const;
    bool isMouseDown(i32 button) const;

private:
    GLFWwindow* m_handle = nullptr;
    i32 m_width = 1280, m_height = 720;
    f32 m_dpiScale = 1.0f;
    bool m_vsync = true;
    EventDispatcher m_events;

    static void glfwErrorCallback(i32 error, const char* desc);
    static void glfwKeyCallback(GLFWwindow* w, i32 key, i32 scancode, i32 action, i32 mods);
    static void glfwCharCallback(GLFWwindow* w, u32 codepoint);
    static void glfwMouseButtonCallback(GLFWwindow* w, i32 button, i32 action, i32 mods);
    static void glfwCursorPosCallback(GLFWwindow* w, f64 x, f64 y);
    static void glfwScrollCallback(GLFWwindow* w, f64 dx, f64 dy);
    static void glfwResizeCallback(GLFWwindow* w, i32 width, i32 height);
    static void glfwFocusCallback(GLFWwindow* w, i32 focused);
    static void glfwDropCallback(GLFWwindow* w, i32 count, const char** paths);
};
