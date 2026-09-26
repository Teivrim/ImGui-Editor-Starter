#include "Framebuffer.h"
#include "Texture.h"
#include "GL.h"
#include "../core/Logger.h"

Framebuffer::~Framebuffer() { destroy(); }

bool Framebuffer::create(u32 width, u32 height, bool withDepth) {
    destroy();
    m_width = width;
    m_height = height;

    m_colorTex = std::make_unique<Texture2D>();
    m_colorTex->create(width, height, nullptr);

    glGenFramebuffers(1, &m_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex->id(), 0);

    if (withDepth) {
        glGenRenderbuffers(1, &m_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (i32)width, (i32)height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        Logger::instance().error("Framebuffer incomplete");
        destroy();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void Framebuffer::destroy() {
    if (m_id) { glDeleteFramebuffers(1, &m_id); m_id = 0; }
    if (m_rbo) { glDeleteRenderbuffers(1, &m_rbo); m_rbo = 0; }
    m_colorTex.reset();
}

void Framebuffer::bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glViewport(0, 0, (i32)m_width, (i32)m_height);
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(u32 width, u32 height) {
    if (width == m_width && height == m_height) return;
    m_width = width;
    m_height = height;

    m_colorTex->create(width, height, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex->id(), 0);

    if (m_rbo) {
        glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (i32)width, (i32)height);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
