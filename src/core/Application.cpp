#include "Application.h"
#include "Logger.h"
#include <GLFW/glfw3.h>
#include "../render/Renderer.h"
#include "../document/Document.h"
#include "../undo/UndoStack.h"
#include "../plugin/PluginManager.h"
#include "../task/TaskScheduler.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

Application::~Application() { shutdown(); }

bool Application::init(const ApplicationDesc& desc) {
    m_desc = desc;
    Logger::instance().info("Initializing " + desc.name + " v" + desc.version);

    m_window = std::make_unique<Window>();
    if (!m_window->create(desc.width, desc.height, desc.name)) {
        Logger::instance().error("Failed to create window");
        return false;
    }
    m_window->setVsync(desc.vsync);

    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->init()) {
        Logger::instance().error("Failed to initialize renderer");
        return false;
    }

    m_undoStack = std::make_unique<UndoStack>(100);
    m_pluginManager = std::make_unique<PluginManager>();
    m_taskScheduler = std::make_unique<TaskScheduler>();
    m_taskScheduler->start(4);

    initImGui();
    onInit();

    Logger::instance().info("Application initialized successfully");
    return true;
}

void Application::run() {
    m_running = true;
    m_timer.start();
    m_lastFrame = Timer::now();

    while (m_running && !m_window->shouldClose()) {
        f64 now = Timer::now();
        m_deltaTime = now - m_lastFrame;
        m_frameTime = m_deltaTime;
        m_lastFrame = now;
        m_fpsCounter.tick();

        m_window->pollEvents();

        newImGuiFrame();

        onUpdate(m_deltaTime);
        onUI();

        m_renderer->beginFrame(0.1f, 0.1f, 0.12f, 1.0f);
        onRender();
        renderImGui();
        m_renderer->endFrame();

        m_window->swapBuffers();
    }
}

void Application::shutdown() {
    if (!m_running) return;
    m_running = false;

    onShutdown();

    if (m_taskScheduler) m_taskScheduler->stop();
    shutdownImGui();
    m_activeDocument.reset();
    m_undoStack.reset();
    m_renderer.reset();
    m_window.reset();

    Logger::instance().info("Application shutdown complete");
}

void Application::setActiveDocument(std::unique_ptr<Document> doc) {
    m_activeDocument = std::move(doc);
    events().dispatch(EventType::DocumentModified);
}

void Application::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    if (m_desc.enableDocking)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_window->handle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void Application::shutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Application::newImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Application::renderImGui() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
}
