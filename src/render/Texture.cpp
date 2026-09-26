#include "Texture.h"
#include "GL.h"
#include "../core/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture2D::~Texture2D() { destroy(); }

bool Texture2D::loadFromFile(const std::string& path) {
    destroy();

    i32 w, h, channels;
    stbi_set_flip_vertically_on_load(true);
    u8* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        Logger::instance().error("Failed to load texture: " + path);
        return false;
    }

    create((u32)w, (u32)h, data);
    stbi_image_free(data);
    Logger::instance().info("Loaded texture: " + path + " (" + std::to_string(w) + "x" + std::to_string(h) + ")");
    return true;
}

void Texture2D::create(u32 width, u32 height, const u8* pixels) {
    destroy();
    m_width = width;
    m_height = height;

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (i32)width, (i32)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::destroy() {
    if (m_id) { glDeleteTextures(1, &m_id); m_id = 0; }
}

void Texture2D::bind(u32 slot) const {
    if (m_id) {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_id);
    }
}

void Texture2D::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
