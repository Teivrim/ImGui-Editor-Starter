#pragma once
#include "../core/Types.h"
#include "../core/Math.h"
#include <string>
#include <vector>
#include <memory>

#include "Layer.h"

class Composition {
public:
    Composition() = default;
    explicit Composition(const std::string& name, u32 width, u32 height);

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    u32 width() const { return m_width; }
    u32 height() const { return m_height; }
    f64 frameRate() const { return m_frameRate; }
    f64 duration() const { return m_duration; }

    void setSize(u32 w, u32 h) { m_width = w; m_height = h; }
    void setFrameRate(f64 fps) { m_frameRate = fps; }
    void setDuration(f64 sec) { m_duration = sec; }

    void addLayer(std::unique_ptr<Layer> layer);
    Layer* layer(u32 index) const;
    u32 layerCount() const { return (u32)m_layers.size(); }
    std::unique_ptr<Layer> removeLayer(u32 index);
    i32 layerIndex(EntityID id) const;
    Layer* findLayer(EntityID id) const;

    u32 selectedLayerIndex() const { return m_selectedLayer; }
    void setSelectedLayer(u32 idx) { m_selectedLayer = idx < m_layers.size() ? idx : m_layers.size() - 1; }
    Layer* selectedLayer() const { return m_selectedLayer < m_layers.size() ? m_layers[m_selectedLayer].get() : nullptr; }

    void render(class Renderer& renderer);

private:
    std::string m_name = "Composition";
    u32 m_width = 1920;
    u32 m_height = 1080;
    f64 m_frameRate = 30.0;
    f64 m_duration = 10.0;
    std::vector<std::unique_ptr<Layer>> m_layers;
    u32 m_selectedLayer = 0;
};
