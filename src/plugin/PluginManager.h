#pragma once
#include "../core/Types.h"
#include <string>
#include <vector>
#include <memory>

#ifdef _WIN32
    #include <windows.h>
    using LibraryHandle = HMODULE;
#else
    #include <dlfcn.h>
    using LibraryHandle = void*;
#endif

#include "Plugin.h"

struct LoadedPlugin {
    std::string path;
    LibraryHandle library = nullptr;
    std::unique_ptr<Plugin> instance;
};

class PluginManager {
public:
    PluginManager() = default;
    ~PluginManager();

    bool loadPlugin(const std::string& path);
    bool unloadPlugin(u32 index);
    void unloadAll();

    Plugin* plugin(u32 index) const;
    u32 pluginCount() const { return (u32)m_plugins.size(); }

    void initAll(void* app);
    void shutdownAll();
    void updateAll(f64 dt);
    void renderUI();

private:
    std::vector<std::unique_ptr<LoadedPlugin>> m_plugins;
};
