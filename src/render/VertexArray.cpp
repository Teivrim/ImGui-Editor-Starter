#include "VertexArray.h"
#include "GL.h"

VertexArray::~VertexArray() { destroy(); }

void VertexArray::destroy() {
    if (m_ibo) { glDeleteBuffers(1, &m_ibo); m_ibo = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_id) { glDeleteVertexArrays(1, &m_id); m_id = 0; }
}

void VertexArray::bind() const {
    glBindVertexArray(m_id);
    if (m_ibo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
}

void VertexArray::unbind() const {
    glBindVertexArray(0);
}

void VertexArray::setVertexBuffer(const f32* data, u32 size, const std::vector<VertexAttrib>& attribs) {
    if (!m_id) glGenVertexArrays(1, &m_id);
    bind();

    if (!m_vbo) glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, (i32)size, data, GL_STATIC_DRAW);

    for (const auto& a : attribs) {
        glEnableVertexAttribArray(a.location);
        glVertexAttribPointer(a.location, a.size, a.type, GL_FALSE, a.stride, (void*)(intptr_t)a.offset);
    }

    unbind();
}

void VertexArray::setIndexBuffer(const u32* data, u32 count) {
    if (!m_id) glGenVertexArrays(1, &m_id);
    bind();

    if (!m_ibo) glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (i32)(count * sizeof(u32)), data, GL_STATIC_DRAW);
    m_indexCount = count;

    unbind();
}
