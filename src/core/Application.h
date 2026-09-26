#pragma once
#include "Types.h"
#include "Math.h"
#include "Window.h"
#include "Timer.h"
#include "Event.h"
#include "../render/Renderer.h"
#include "../document/Document.h"
#include "../undo/UndoStack.h"
#include "../plugin/PluginManager.h"
#include "../task/TaskScheduler.h"
#include <string>

class DockingSpace;
#include <vector>
#include <memory>

struct ApplicationDesc {
    std::string name = "Editor";
    std::string version = "1.0.0";
    i32 width = 1280;
    i32 height = 720;
    bool vsync = true;
    bool enableDocking = true;
    std::string assetsPath = "assets";
};

class Application {
public:
    Application() = default;
    virtual ~Application();

    bool init(const ApplicationDesc& desc);
    void run();
    void shutdown();

    Window& window() { return *m_window; }
    Renderer& renderer() { return *m_renderer; }
    EventDispatcher& events() { return m_window->events(); }
    UndoStack& undoStack() { return *m_undoStack; }
    PluginManager& pluginManager() { return *m_pluginManager; }
    TaskScheduler& taskScheduler() { return *m_taskScheduler; }

    Document* activeDocument() const { return m_activeDocument.get(); }
    void setActiveDocument(std::unique_ptr<Document> doc);

    DockingSpace* dockingSpace() const { return m_dockingSpace; }
    void setDockingSpace(DockingSpace* ds) { m_dockingSpace = ds; }

    const ApplicationDesc& desc() const { return m_desc; }
    Timer& timer() { return m_timer; }
    f64 deltaTime() const { return m_deltaTime; }
    f64 frameTime() const { return m_frameTime; }
    u32 fps() const { return m_fpsCounter.fps(); }

    bool isRunning() const { return m_running; }

    static Application& get() {
        static Application instance;
        return instance;
    }

protected:
    virtual void onInit() {}
    virtual void onUpdate(f64 dt) {}
    virtual void onRender() {}
    virtual void onUI() {}
    virtual void onShutdown() {}

private:
    ApplicationDesc m_desc;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<UndoStack> m_undoStack;
    std::unique_ptr<PluginManager> m_pluginManager;
    std::unique_ptr<TaskScheduler> m_taskScheduler;
    std::unique_ptr<Document> m_activeDocument;
    DockingSpace* m_dockingSpace = nullptr;

    Timer m_timer;
    FPSCounter m_fpsCounter;
    f64 m_deltaTime = 0;
    f64 m_frameTime = 0;
    f64 m_lastFrame = 0;
    bool m_running = false;

    void initImGui();
    void shutdownImGui();
    void newImGuiFrame();
    void renderImGui();
};

#define APP_CREATE() Application::get()
