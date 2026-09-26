#include "Composition.h"
#include "Layer.h"
#include "../render/Renderer.h"

Composition::Composition(const std::string& name, u32 width, u32 height)
    : m_name(name), m_width(width), m_height(height) {}

void Composition::addLayer(std::unique_ptr<Layer> layer) {
    m_layers.push_back(std::move(layer));
}

Layer* Composition::layer(u32 index) const {
    return index < m_layers.size() ? m_layers[index].get() : nullptr;
}

std::unique_ptr<Layer> Composition::removeLayer(u32 index) {
    if (index >= m_layers.size()) return nullptr;
    auto it = m_layers.begin() + index;
    auto ptr = std::move(*it);
    m_layers.erase(it);
    return ptr;
}

i32 Composition::layerIndex(EntityID id) const {
    for (u32 i = 0; i < m_layers.size(); i++)
        if (m_layers[i]->id() == id) return (i32)i;
    return -1;
}

Layer* Composition::findLayer(EntityID id) const {
    for (auto& l : m_layers) {
        if (l->id() == id) return l.get();
        if (l->type() == LayerType::Group) {
            auto* g = static_cast<GroupLayer*>(l.get());
            for (u32 i = 0; i < g->layerCount(); i++) {
                if (g->layer(i)->id() == id) return g->layer(i);
            }
        }
    }
    return nullptr;
}

void Composition::render(Renderer& renderer) {
    auto proj = Mat4::ortho(0, (f32)m_width, (f32)m_height, 0, -1, 1);
    renderer.setProjection(proj);

    for (auto& layer : m_layers)
        layer->render(renderer, Mat4::identity());
}
