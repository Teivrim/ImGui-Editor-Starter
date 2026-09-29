#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "VertexArray.h"
#include "Mesh.h"
#include "GL.h"
#include "../core/Logger.h"

#include <GLFW/glfw3.h>

static const f32 QUAD_VERTS[] = {
    0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 1, 1, 0, 1,
    1, 1, 0, 1, 1, 1, 1,
    0, 1, 0, 0, 1, 1, 1,
};
static const u32 QUAD_INDICES[] = {0, 1, 2, 0, 2, 3};

Renderer::~Renderer() = default;

bool Renderer::init() {
    // GLAD 2 ждёт GLADloadfunc — это указатель на функцию,
// возвращающую указатель на функцию. Старый каст
// (void* (*)(const char*)) был рассчитан на GLAD 1 и не подходит:
// «invalid conversion ... to GLADloadfunc».
gladLoadGL((GLADloadfunc)glfwGetProcAddress);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    Logger::instance().info("OpenGL " + std::string((const char*)glGetString(GL_VERSION)));
    Logger::instance().info("GPU: " + std::string((const char*)glGetString(GL_RENDERER)));

    m_quadVA = std::make_unique<VertexArray>();
    m_quadVA->bind();
    m_quadVA->setVertexBuffer(QUAD_VERTS, sizeof(QUAD_VERTS), {
        {0, 3, GL_FLOAT, 7 * sizeof(f32), 0},
        {1, 4, GL_FLOAT, 7 * sizeof(f32), 3 * sizeof(f32)}
    });
    m_quadVA->setIndexBuffer(QUAD_INDICES, 6);
    m_quadVA->unbind();

    auto vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
uniform mat4 uProjection;
uniform mat4 uTransform;
out vec4 vColor;
void main() {
    gl_Position = uProjection * uTransform * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

    auto fragSrc = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;
uniform vec4 uColor = vec4(1,1,1,1);
void main() {
    FragColor = vColor * uColor;
}
)";

    createShader("basic", vertSrc, fragSrc);

    auto texVertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
uniform mat4 uProjection;
uniform mat4 uTransform;
out vec4 vColor;
out vec2 vTexCoord;
void main() {
    gl_Position = uProjection * uTransform * vec4(aPos, 1.0);
    vColor = aColor;
    vTexCoord = aPos.xy;
}
)";

    auto texFragSrc = R"(
#version 330 core
in vec4 vColor;
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D uTexture;
uniform vec4 uTint = vec4(1,1,1,1);
void main() {
    FragColor = texture(uTexture, vTexCoord) * vColor * uTint;
}
)";

    createShader("texture", texVertSrc, texFragSrc);

    glGenVertexArrays(1, &m_defaultVAO);

    return true;
}

void Renderer::shutdown() {
    m_shaders.clear();
    m_textures.clear();
    m_quadVA.reset();
    glDeleteVertexArrays(1, &m_defaultVAO);
}

void Renderer::beginFrame(f32 r, f32 g, f32 b, f32 a) {
    clear(r, g, b, a);
    m_transformStack = {Mat4::identity()};
    m_currentBlend = BlendMode::Normal;
    setBlendMode(BlendMode::Normal);
    unbindFramebuffer();
}

void Renderer::endFrame() {
    glBindVertexArray(0);
}

void Renderer::clear(f32 r, f32 g, f32 b, f32 a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setViewport(i32 x, i32 y, i32 w, i32 h) {
    glViewport(x, y, w, h);
}

void Renderer::setBlendMode(BlendMode mode) {
    if (mode == m_currentBlend) return;
    m_currentBlend = mode;

    if (mode == BlendMode::Normal) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else if (mode == BlendMode::Add) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    } else if (mode == BlendMode::Multiply) {
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
    } else if (mode == BlendMode::Screen) {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void Renderer::drawMesh(const Mesh& mesh, Shader& shader) {
    shader.bind();
    shader.setMat4("uProjection", m_projection);
    shader.setMat4("uTransform", m_transformStack.back());

    mesh.va().bind();
    glDrawElements(GL_TRIANGLES, mesh.indexCount(), GL_UNSIGNED_INT, nullptr);
    mesh.va().unbind();
}

void Renderer::drawFullscreenQuad(Shader& shader) {
    shader.bind();
    shader.setMat4("uProjection", Mat4::ortho(-1, 1, -1, 1, -1, 1));
    shader.setMat4("uTransform", Mat4::identity());

    m_quadVA->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    m_quadVA->unbind();
}

void Renderer::drawRect(const Rect& rect, const ColorRGBA& color) {
    auto* shader = getShader("basic");
    if (!shader) return;

    Mat4 transform = Mat4::translate(rect.x, rect.y, 0) *
                     Mat4::scale(rect.w, rect.h, 1);

    pushTransform(transform);
    shader->bind();
    shader->setMat4("uProjection", m_projection);
    shader->setMat4("uTransform", m_transformStack.back());
    shader->setVec4("uColor", Vec4{color.r, color.g, color.b, color.a});

    m_quadVA->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    m_quadVA->unbind();
    popTransform();
}

void Renderer::drawTexture(Texture2D& tex, const Rect& dst, const Rect& src, const ColorRGBA& tint) {
    auto* shader = getShader("texture");
    if (!shader) return;

    tex.bind(0);
    Mat4 transform = Mat4::translate(dst.x, dst.y, 0) *
                     Mat4::scale(dst.w, dst.h, 1);

    pushTransform(transform);
    shader->bind();
    shader->setMat4("uProjection", m_projection);
    shader->setMat4("uTransform", m_transformStack.back());
    shader->setInt("uTexture", 0);
    shader->setVec4("uTint", Vec4{tint.r, tint.g, tint.b, tint.a});

    m_quadVA->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    m_quadVA->unbind();
    popTransform();
}

Shader* Renderer::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    return it != m_shaders.end() ? it->second.get() : nullptr;
}

Shader* Renderer::loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) {
    auto shader = std::make_unique<Shader>();
    if (!shader->loadFromFile(vertPath, fragPath)) return nullptr;
    auto* ptr = shader.get();
    m_shaders[name] = std::move(shader);
    return ptr;
}

Shader* Renderer::createShader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc) {
    auto shader = std::make_unique<Shader>();
    if (!shader->loadFromSource(vertSrc, fragSrc)) return nullptr;
    auto* ptr = shader.get();
    m_shaders[name] = std::move(shader);
    return ptr;
}

Texture2D* Renderer::getTexture(const std::string& name) {
    auto it = m_textures.find(name);
    return it != m_textures.end() ? it->second.get() : nullptr;
}

Texture2D* Renderer::loadTexture(const std::string& name, const std::string& path) {
    auto tex = std::make_unique<Texture2D>();
    if (!tex->loadFromFile(path)) return nullptr;
    auto* ptr = tex.get();
    m_textures[name] = std::move(tex);
    return ptr;
}

Texture2D* Renderer::createTexture(const std::string& name, u32 width, u32 height, const u8* data) {
    auto tex = std::make_unique<Texture2D>();
    tex->create(width, height, data);
    auto* ptr = tex.get();
    m_textures[name] = std::move(tex);
    return ptr;
}

void Renderer::bindFramebuffer(Framebuffer* fb) {
    if (fb) {
        fb->bind();
        m_boundFB = fb;
    } else {
        unbindFramebuffer();
    }
}

void Renderer::unbindFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_boundFB = nullptr;
}

Vec2 Renderer::worldToScreen(const Vec2& world) const {
    Vec4 p = Vec4{world.x, world.y, 0, 1};
    Mat4 vp = m_projection * m_transformStack.back();
    Vec4 r;
    for (i32 i = 0; i < 4; i++)
        for (i32 j = 0; j < 4; j++)
            (&r.x)[i] += vp.m[i * 4 + j] * (&p.x)[j];
    return {r.x / r.w, r.y / r.w};
}

Vec2 Renderer::screenToWorld(const Vec2& screen) const {
    return screen;
}

void Renderer::pushTransform(const Mat4& mat) {
    m_transformStack.push_back(m_transformStack.back() * mat);
}

void Renderer::popTransform() {
    if (m_transformStack.size() > 1)
        m_transformStack.pop_back();
}
