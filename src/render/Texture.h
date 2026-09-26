#pragma once
#include "../core/Types.h"
#include <string>

class Texture2D {
public:
    Texture2D() = default;
    ~Texture2D();

    Texture2D(Texture2D&& o) noexcept : m_id(o.m_id), m_width(o.m_width), m_height(o.m_height) { o.m_id = 0; }
    Texture2D& operator=(Texture2D&& o) noexcept {
        if (this != &o) { destroy(); m_id = o.m_id; m_width = o.m_width; m_height = o.m_height; o.m_id = 0; }
        return *this;
    }

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    bool loadFromFile(const std::string& path);
    void create(u32 width, u32 height, const u8* pixels);
    void destroy();

    void bind(u32 slot = 0) const;
    void unbind() const;

    u32 id() const { return m_id; }
    u32 width() const { return m_width; }
    u32 height() const { return m_height; }

private:
    u32 m_id = 0;
    u32 m_width = 0;
    u32 m_height = 0;
};
