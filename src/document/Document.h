#pragma once
#include "../core/Types.h"
#include "../core/Math.h"
#include "Composition.h"
#include <string>
#include <vector>
#include <memory>

class Document {
public:
    Document() = default;
    explicit Document(const std::string& name);

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    const std::string& filePath() const { return m_filePath; }
    void setFilePath(const std::string& path) { m_filePath = path; }

    bool modified() const { return m_modified; }
    void setModified(bool m) { m_modified = m; }

    u32 compositionCount() const { return (u32)m_compositions.size(); }
    Composition* composition(u32 index) const;
    void addComposition(std::unique_ptr<Composition> comp);
    std::unique_ptr<Composition> removeComposition(u32 index);
    Composition* activeComposition() const { return m_activeComp; }
    void setActiveComposition(u32 index);
    void setActiveComposition(Composition* comp);

    bool save(const std::string& path);
    bool load(const std::string& path);

private:
    std::string m_name = "Untitled";
    std::string m_filePath;
    bool m_modified = false;
    std::vector<std::unique_ptr<Composition>> m_compositions;
    Composition* m_activeComp = nullptr;
};
