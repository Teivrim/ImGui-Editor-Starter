#pragma once
#include "../core/Types.h"
#include "../core/Math.h"
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(Shader&& o) noexcept : m_id(o.m_id), m_uniforms(std::move(o.m_uniforms)) { o.m_id = 0; }
    Shader& operator=(Shader&& o) noexcept {
        if (this != &o) { destroy(); m_id = o.m_id; m_uniforms = std::move(o.m_uniforms); o.m_id = 0; }
        return *this;
    }

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool loadFromFile(const std::string& vertPath, const std::string& fragPath);
    bool loadFromSource(const std::string& vertSrc, const std::string& fragSrc);

    void bind() const;
    void unbind() const;
    u32 id() const { return m_id; }

    void setInt(const std::string& name, i32 value);
    void setFloat(const std::string& name, f32 value);
    void setVec2(const std::string& name, const Vec2& value);
    void setVec3(const std::string& name, const Vec3& value);
    void setVec4(const std::string& name, const Vec4& value);
    void setMat4(const std::string& name, const Mat4& value);

private:
    u32 m_id = 0;
    mutable std::unordered_map<std::string, i32> m_uniforms;

    void destroy();
    i32 getLocation(const std::string& name) const;
    static u32 compileShader(u32 type, const std::string& source);
    static bool checkLink(u32 program);
};
