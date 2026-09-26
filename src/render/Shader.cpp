#include "Shader.h"
#include "GL.h"
#include "../core/Logger.h"
#include <fstream>
#include <sstream>

Shader::~Shader() { destroy(); }

void Shader::destroy() {
    if (m_id) { glDeleteProgram(m_id); m_id = 0; }
}

bool Shader::loadFromFile(const std::string& vertPath, const std::string& fragPath) {
    auto readFile = [](const std::string& path) -> std::string {
        std::ifstream f(path);
        if (!f.is_open()) return {};
        return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    };
    return loadFromSource(readFile(vertPath), readFile(fragPath));
}

bool Shader::loadFromSource(const std::string& vertSrc, const std::string& fragSrc) {
    if (vertSrc.empty() || fragSrc.empty()) {
        Logger::instance().error("Shader source is empty");
        return false;
    }

    u32 vs = compileShader(GL_VERTEX_SHADER, vertSrc);
    u32 fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!vs || !fs) return false;

    m_id = glCreateProgram();
    glAttachShader(m_id, vs);
    glAttachShader(m_id, fs);
    glLinkProgram(m_id);

    if (!checkLink(m_id)) {
        glDeleteProgram(m_id);
        m_id = 0;
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

void Shader::bind() const { glUseProgram(m_id); }
void Shader::unbind() const { glUseProgram(0); }

i32 Shader::getLocation(const std::string& name) const {
    auto it = m_uniforms.find(name);
    if (it != m_uniforms.end()) return it->second;
    i32 loc = glGetUniformLocation(m_id, name.c_str());
    m_uniforms[name] = loc;
    return loc;
}

void Shader::setInt(const std::string& name, i32 value) {
    glUniform1i(getLocation(name), value);
}

void Shader::setFloat(const std::string& name, f32 value) {
    glUniform1f(getLocation(name), value);
}

void Shader::setVec2(const std::string& name, const Vec2& value) {
    glUniform2f(getLocation(name), value.x, value.y);
}

void Shader::setVec3(const std::string& name, const Vec3& value) {
    glUniform3f(getLocation(name), value.x, value.y, value.z);
}

void Shader::setVec4(const std::string& name, const Vec4& value) {
    glUniform4f(getLocation(name), value.x, value.y, value.z, value.w);
}

void Shader::setMat4(const std::string& name, const Mat4& value) {
    glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, value.data());
}

u32 Shader::compileShader(u32 type, const std::string& source) {
    u32 shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    i32 success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        Logger::instance().error(std::string("Shader compile error: ") + log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::checkLink(u32 program) {
    i32 success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        Logger::instance().error(std::string("Program link error: ") + log);
        return false;
    }
    return true;
}
