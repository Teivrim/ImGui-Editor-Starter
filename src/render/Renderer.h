#pragma once
#include "../core/Types.h"
#include "../core/Math.h"
#include <unordered_map>
#include <string>

#include "Shader.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "VertexArray.h"
#include "Mesh.h"

class Renderer {
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init();
    void shutdown();
    void beginFrame(f32 r, f32 g, f32 b, f32 a);
    void endFrame();

    void clear(f32 r, f32 g, f32 b, f32 a);
    void setViewport(i32 x, i32 y, i32 w, i32 h);
    void setBlendMode(BlendMode mode);

    void drawMesh(const Mesh& mesh, Shader& shader);
    void drawFullscreenQuad(Shader& shader);
    void drawRect(const Rect& rect, const ColorRGBA& color);
    void drawTexture(Texture2D& tex, const Rect& dst, const Rect& src = {}, const ColorRGBA& tint = {1,1,1,1});

    Shader* getShader(const std::string& name);
    Shader* loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);
    Shader* createShader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);

    Texture2D* getTexture(const std::string& name);
    Texture2D* loadTexture(const std::string& name, const std::string& path);
    Texture2D* createTexture(const std::string& name, u32 width, u32 height, const u8* data);

    void bindFramebuffer(Framebuffer* fb);
    void unbindFramebuffer();

    Vec2 worldToScreen(const Vec2& world) const;
    Vec2 screenToWorld(const Vec2& screen) const;

    void pushTransform(const Mat4& mat);
    void popTransform();
    Mat4& currentTransform() { return m_transformStack.back(); }
    const Mat4& projection() const { return m_projection; }
    void setProjection(const Mat4& proj) { m_projection = proj; }

private:
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::unique_ptr<Texture2D>> m_textures;
    std::unique_ptr<VertexArray> m_quadVA;
    std::unique_ptr<Mesh> m_quadMesh;
    Framebuffer* m_boundFB = nullptr;
    std::vector<Mat4> m_transformStack{Mat4::identity()};
    Mat4 m_projection;
    BlendMode m_currentBlend = BlendMode::Normal;
    u32 m_defaultVAO = 0;
};
