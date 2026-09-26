#pragma once
#include "../core/Types.h"
#include <vector>
#include <memory>

class Panel;

class DockingSpace {
public:
    DockingSpace() = default;

    void addPanel(std::unique_ptr<Panel> panel);
    Panel* panel(u32 index) const;
    u32 panelCount() const { return (u32)m_panels.size(); }
    void removePanel(u32 index);

    template<typename T>
    T* findPanel() const {
        for (auto& p : m_panels) {
            if (auto* casted = dynamic_cast<T*>(p.get()))
                return casted;
        }
        return nullptr;
    }

    void render();

    void setDockSpaceName(const std::string& name) { m_dockspaceName = name; }
    const std::string& dockSpaceName() const { return m_dockspaceName; }

private:
    std::string m_dockspaceName = "MainDockspace";
    std::vector<std::unique_ptr<Panel>> m_panels;
};
