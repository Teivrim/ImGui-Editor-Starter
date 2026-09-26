#pragma once
#include "../core/Types.h"
#include <string>

#ifdef _WIN32
    #define PLUGIN_EXPORT __declspec(dllexport)
#else
    #define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
    typedef struct PluginInfo {
        const char* name;
        const char* version;
        const char* description;
        const char* author;
    } PluginInfo;

    typedef PluginInfo* (*GetPluginInfoFunc)();
    typedef bool (*InitPluginFunc)(void* app);
    typedef void (*ShutdownPluginFunc)();
}

class Plugin {
public:
    virtual ~Plugin() = default;

    virtual const char* name() const = 0;
    virtual const char* version() const = 0;
    virtual const char* description() const = 0;
    virtual const char* author() const = 0;

    virtual bool init(void* app) = 0;
    virtual void shutdown() = 0;
    virtual void onUI() {}
    virtual void onUpdate(f64 dt) {}
};
