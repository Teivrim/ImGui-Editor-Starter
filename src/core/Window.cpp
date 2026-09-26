#include "Window.h"
#include "Logger.h"
#include <GLFW/glfw3.h>
#include <cstdlib>

Window::~Window() { destroy(); }

bool Window::create(i32 width, i32 height, const std::string& title) {
    m_width = width;
    m_height = height;

    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        Logger::instance().error("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    m_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_handle) {
        Logger::instance().error("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_handle);
    glfwSwapInterval(m_vsync ? 1 : 0);

    m_dpiScale = 1.0f;
    f32 xscale, yscale;
    glfwGetWindowContentScale(m_handle, &xscale, &yscale);
    m_dpiScale = (xscale + yscale) / 2.0f;

    glfwSetWindowUserPointer(m_handle, this);
    glfwSetKeyCallback(m_handle, glfwKeyCallback);
    glfwSetCharCallback(m_handle, glfwCharCallback);
    glfwSetMouseButtonCallback(m_handle, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(m_handle, glfwCursorPosCallback);
    glfwSetScrollCallback(m_handle, glfwScrollCallback);
    glfwSetWindowSizeCallback(m_handle, glfwResizeCallback);
    glfwSetWindowFocusCallback(m_handle, glfwFocusCallback);
    glfwSetDropCallback(m_handle, glfwDropCallback);

    Logger::instance().info("Window created: " + std::to_string(width) + "x" + std::to_string(height));
    return true;
}

void Window::destroy() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
    }
    glfwTerminate();
}

void Window::pollEvents() { glfwPollEvents(); }
void Window::swapBuffers() { glfwSwapBuffers(m_handle); }
bool Window::shouldClose() const { return glfwWindowShouldClose(m_handle); }
void Window::close() { glfwSetWindowShouldClose(m_handle, GLFW_TRUE); }

void Window::setVsync(bool enabled) {
    m_vsync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(m_handle, title.c_str());
}

Vec2 Window::mousePos() const {
    f64 x, y;
    glfwGetCursorPos(m_handle, &x, &y);
    return {(f32)x, (f32)y};
}

bool Window::isKeyDown(i32 key) const {
    return glfwGetKey(m_handle, key) == GLFW_PRESS;
}

bool Window::isMouseDown(i32 button) const {
    return glfwGetMouseButton(m_handle, button) == GLFW_PRESS;
}

void Window::glfwErrorCallback(i32 error, const char* desc) {
    Logger::instance().error("GLFW error [" + std::to_string(error) + "]: " + desc);
}

Window* getWindow(GLFWwindow* w) {
    return static_cast<Window*>(glfwGetWindowUserPointer(w));
}

void Window::glfwKeyCallback(GLFWwindow* w, i32 key, i32, i32 action, i32 mods) {
    auto* win = getWindow(w);
    if (!win) return;
    Event e;
    e.type = (action == GLFW_PRESS || action == GLFW_REPEAT) ? EventType::KeyDown : EventType::KeyUp;
    e.key.key = key;
    e.key.mods = mods;
    win->m_events.dispatch(e);
}

void Window::glfwCharCallback(GLFWwindow* w, u32 codepoint) {
    auto* win = getWindow(w);
    if (!win) return;
    Event e(EventType::KeyChar);
    e.character.codepoint = codepoint;
    win->m_events.dispatch(e);
}

void Window::glfwMouseButtonCallback(GLFWwindow* w, i32 button, i32 action, i32 mods) {
    auto* win = getWindow(w);
    if (!win) return;
    Event e;
    e.type = (action == GLFW_PRESS) ? EventType::MouseDown : EventType::MouseUp;
    f64 x, y; glfwGetCursorPos(w, &x, &y);
    e.mouseButton.x = (f32)x;
    e.mouseButton.y = (f32)y;
    e.mouseButton.button = button;
    e.mouseButton.mods = mods;
    win->m_events.dispatch(e);
}

void Window::glfwCursorPosCallback(GLFWwindow* w, f64 x, f64 y) {
    auto* win = getWindow(w);
    if (!win) return;
    Event e(EventType::MouseMove);
    e.mouseMove.x = (f32)x;
    e.mouseMove.y = (f32)y;
    win->m_events.dispatch(e);
}

void Window::glfwScrollCallback(GLFWwindow* w, f64 dx, f64 dy) {
    auto* win = getWindow(w);
    if (!win) return;
    Event e(EventType::MouseScroll);
    e.scroll.dx = (f32)dx;
    e.scroll.dy = (f32)dy;
    win->m_events.dispatch(e);
}

void Window::glfwResizeCallback(GLFWwindow* w, i32 width, i32 height) {
    auto* win = getWindow(w);
    if (!win) return;
    win->m_width = width;
    win->m_height = height;
    Event e(EventType::WindowResize);
    e.resize.w = width;
    e.resize.h = height;
    win->m_events.dispatch(e);
}

void Window::glfwFocusCallback(GLFWwindow* w, i32 focused) {
    auto* win = getWindow(w);
    if (!win) return;
    win->m_events.dispatch(focused ? EventType::WindowFocus : EventType::None);
}

void Window::glfwDropCallback(GLFWwindow* w, i32 count, const char** paths) {
    auto* win = getWindow(w);
    if (!win) return;
    std::vector<std::string> files;
    for (i32 i = 0; i < count; i++) files.emplace_back(paths[i]);
    Event e(EventType::FileDrop);
    e.fileDrop.files = &files;
    win->m_events.dispatch(e);
}
