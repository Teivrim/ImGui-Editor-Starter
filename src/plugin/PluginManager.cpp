#include "PluginManager.h"
#include "Plugin.h"
#include "../core/Logger.h"

PluginManager::~PluginManager() { unloadAll(); }

bool PluginManager::loadPlugin(const std::string& path) {
    auto plugin = std::make_unique<LoadedPlugin>();
    plugin->path = path;

#ifdef _WIN32
    plugin->library = LoadLibraryA(path.c_str());
    if (!plugin->library) {
        Logger::instance().error("Failed to load plugin: " + path + " (error " +
                                 std::to_string(GetLastError()) + ")");
        return false;
    }

    auto getInfo = (GetPluginInfoFunc)GetProcAddress(plugin->library, "GetPluginInfo");
    auto createPlugin = (Plugin * (*)())GetProcAddress(plugin->library, "CreatePlugin");
#else
    plugin->library = dlopen(path.c_str(), RTLD_NOW);
    if (!plugin->library) {
        Logger::instance().error("Failed to load plugin: " + path + " (" + dlerror() + ")");
        return false;
    }

    auto getInfo = (GetPluginInfoFunc)dlsym(plugin->library, "GetPluginInfo");
    auto createPlugin = (Plugin * (*)())dlsym(plugin->library, "CreatePlugin");
#endif

    if (!createPlugin) {
        Logger::instance().error("Plugin missing CreatePlugin: " + path);
        unloadPlugin((u32)m_plugins.size());
        return false;
    }

    plugin->instance.reset(createPlugin());

    if (getInfo) {
        auto* info = getInfo();
        Logger::instance().info("Loaded plugin: " + std::string(info->name) +
                                " v" + std::string(info->version));
    }

    m_plugins.push_back(std::move(plugin));
    return true;
}

bool PluginManager::unloadPlugin(u32 index) {
    if (index >= m_plugins.size()) return false;
    auto& p = m_plugins[index];
    if (p->instance) p->instance->shutdown();
    p->instance.reset();
    if (p->library) {
#ifdef _WIN32
        FreeLibrary(p->library);
#else
        dlclose(p->library);
#endif
    }
    m_plugins.erase(m_plugins.begin() + index);
    return true;
}

void PluginManager::unloadAll() {
    for (u32 i = (u32)m_plugins.size(); i > 0; i--)
        unloadPlugin(i - 1);
}

Plugin* PluginManager::plugin(u32 index) const {
    return index < m_plugins.size() ? m_plugins[index]->instance.get() : nullptr;
}

void PluginManager::initAll(void* app) {
    for (auto& p : m_plugins) {
        if (p->instance && !p->instance->init(app))
            Logger::instance().error(std::string("Plugin init failed: ") + p->instance->name());
    }
}

void PluginManager::shutdownAll() {
    for (auto& p : m_plugins)
        if (p->instance) p->instance->shutdown();
}

void PluginManager::updateAll(f64 dt) {
    for (auto& p : m_plugins)
        if (p->instance) p->instance->onUpdate(dt);
}

void PluginManager::renderUI() {
    for (auto& p : m_plugins)
        if (p->instance) p->instance->onUI();
}
