#pragma once
#include "../core/Types.h"
#include <vector>

struct VertexAttrib {
    u32 location;
    i32 size;
    u32 type;
    i32 stride;
    i32 offset;
};

class VertexArray {
public:
    VertexArray() = default;
    ~VertexArray();

    VertexArray(VertexArray&& o) noexcept
        : m_id(o.m_id), m_vbo(o.m_vbo), m_ibo(o.m_ibo), m_indexCount(o.m_indexCount) { o.m_id = 0; o.m_vbo = 0; o.m_ibo = 0; }
    VertexArray& operator=(VertexArray&& o) noexcept {
        if (this != &o) { destroy(); m_id = o.m_id; m_vbo = o.m_vbo; m_ibo = o.m_ibo; m_indexCount = o.m_indexCount; o.m_id = 0; o.m_vbo = 0; o.m_ibo = 0; }
        return *this;
    }

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    void bind() const;
    void unbind() const;

    void setVertexBuffer(const f32* data, u32 size, const std::vector<VertexAttrib>& attribs);
    void setIndexBuffer(const u32* data, u32 count);

    u32 indexCount() const { return m_indexCount; }

private:
    u32 m_id = 0, m_vbo = 0, m_ibo = 0;
    u32 m_indexCount = 0;

    void destroy();
};
