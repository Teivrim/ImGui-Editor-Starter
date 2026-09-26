#include "Document.h"
#include "../core/Logger.h"
#include <fstream>
#include <sstream>

Document::Document(const std::string& name) : m_name(name) {}

Composition* Document::composition(u32 index) const {
    return index < m_compositions.size() ? m_compositions[index].get() : nullptr;
}

void Document::addComposition(std::unique_ptr<Composition> comp) {
    m_compositions.push_back(std::move(comp));
    if (!m_activeComp) m_activeComp = m_compositions.back().get();
    m_modified = true;
}

std::unique_ptr<Composition> Document::removeComposition(u32 index) {
    if (index >= m_compositions.size()) return nullptr;
    auto it = m_compositions.begin() + index;
    auto ptr = std::move(*it);
    m_compositions.erase(it);

    if (m_activeComp == ptr.get()) {
        m_activeComp = m_compositions.empty() ? nullptr : m_compositions[0].get();
    }
    m_modified = true;
    return ptr;
}

void Document::setActiveComposition(u32 index) {
    if (index < m_compositions.size()) m_activeComp = m_compositions[index].get();
}

void Document::setActiveComposition(Composition* comp) {
    for (auto& c : m_compositions) {
        if (c.get() == comp) { m_activeComp = comp; return; }
    }
}

bool Document::save(const std::string& path) {
    m_filePath = path;
    Logger::instance().info("Saving document: " + path);

    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) {
        Logger::instance().error("Failed to save: " + path);
        return false;
    }

    u32 magic = 0x45444F43;
    u32 version = 1;
    f.write((const char*)&magic, sizeof(magic));
    f.write((const char*)&version, sizeof(version));

    u32 nameLen = (u32)m_name.size();
    f.write((const char*)&nameLen, sizeof(nameLen));
    f.write(m_name.data(), nameLen);

    u32 compCount = (u32)m_compositions.size();
    f.write((const char*)&compCount, sizeof(compCount));

    m_modified = false;
    return true;
}

bool Document::load(const std::string& path) {
    Logger::instance().info("Loading document: " + path);
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) {
        Logger::instance().error("Failed to load: " + path);
        return false;
    }

    u32 magic, version;
    f.read((char*)&magic, sizeof(magic));
    f.read((char*)&version, sizeof(version));

    if (magic != 0x45444F43) {
        Logger::instance().error("Invalid file format: " + path);
        return false;
    }

    u32 nameLen;
    f.read((char*)&nameLen, sizeof(nameLen));
    m_name.resize(nameLen);
    f.read(m_name.data(), nameLen);

    m_filePath = path;
    m_modified = false;
    return true;
}
