#pragma once
#include "Types.h"
#include "Math.h"
#include <any>
#include <unordered_map>
#include <vector>
#include <functional>

enum class EventType : u32 {
    None,
    WindowClose,
    WindowResize,
    WindowFocus,
    WindowMove,
    KeyDown,
    KeyUp,
    KeyChar,
    MouseMove,
    MouseDown,
    MouseUp,
    MouseScroll,
    MouseEnter,
    DragDrop,
    FileDrop,
    DocumentModified,
    SelectionChanged,
    ToolChanged,
    LayerChanged,
    ProjectChanged,
    Custom
};

struct Event {
    EventType type = EventType::None;
    bool handled = false;

    union {
        struct { i32 key, mods; } key;
        struct { f32 x, y; } mouseMove;
        struct { f32 x, y; i32 button, mods; } mouseButton;
        struct { f32 dx, dy; } scroll;
        struct { i32 w, h; } resize;
        struct { i32 x, y; } move;
        struct { u32 codepoint; } character;
        struct { std::vector<std::string>* files; } fileDrop;
    };

    Event() = default;
    explicit Event(EventType t) : type(t) {}
};

using EventCallback = std::function<void(Event&)>;

class EventDispatcher {
public:
    using Token = u64;

    Token subscribe(EventType type, EventCallback cb) {
        Token t = ++m_nextToken;
        m_handlers[type].push_back({t, std::move(cb)});
        return t;
    }

    void unsubscribe(Token token) {
        for (auto& [type, handlers] : m_handlers) {
            auto it = std::remove_if(handlers.begin(), handlers.end(),
                [token](const auto& h) { return h.first == token; });
            handlers.erase(it, handlers.end());
        }
    }

    void dispatch(Event& event) {
        auto it = m_handlers.find(event.type);
        if (it != m_handlers.end()) {
            for (auto& [_, cb] : it->second) {
                cb(event);
                if (event.handled) break;
            }
        }
    }

    void dispatch(EventType type) {
        Event e(type);
        dispatch(e);
    }

private:
    std::unordered_map<EventType, std::vector<std::pair<Token, EventCallback>>> m_handlers;
    Token m_nextToken = 0;
};
