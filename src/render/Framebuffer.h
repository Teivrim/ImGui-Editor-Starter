#pragma once
#include "../core/Types.h"

class Texture2D;

class Framebuffer {
public:
    Framebuffer() = default;
    ~Framebuffer();

    Framebuffer(Framebuffer&& o) noexcept
        : m_id(o.m_id), m_rbo(o.m_rbo), m_width(o.m_width), m_height(o.m_height), m_colorTex(std::move(o.m_colorTex)) { o.m_id = 0; o.m_rbo = 0; }
    Framebuffer& operator=(Framebuffer&& o) noexcept {
        if (this != &o) { destroy(); m_id = o.m_id; m_rbo = o.m_rbo; m_width = o.m_width; m_height = o.m_height; m_colorTex = std::move(o.m_colorTex); o.m_id = 0; o.m_rbo = 0; }
        return *this;
    }

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    bool create(u32 width, u32 height, bool withDepth = true);
    void destroy();
    void bind() const;
    void unbind() const;

    void resize(u32 width, u32 height);

    u32 id() const { return m_id; }
    u32 width() const { return m_width; }
    u32 height() const { return m_height; }
    Texture2D* colorTexture() { return m_colorTex.get(); }
    const Texture2D* colorTexture() const { return m_colorTex.get(); }

private:
    u32 m_id = 0;
    u32 m_rbo = 0;
    u32 m_width = 0;
    u32 m_height = 0;
    std::unique_ptr<Texture2D> m_colorTex;
};
