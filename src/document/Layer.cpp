#include "Layer.h"
#include "../render/Texture.h"
#include "../render/Renderer.h"
#include "../effect/Effect.h"

#include <cstring>

EntityID Layer::s_nextId = 1;

void Layer::addEffect(std::unique_ptr<Effect> effect) {
    m_effects.push_back(std::move(effect));
}

Effect* Layer::effect(u32 index) const {
    return index < m_effects.size() ? m_effects[index].get() : nullptr;
}

void Layer::removeEffect(u32 index) {
    if (index < m_effects.size()) m_effects.erase(m_effects.begin() + index);
}

void PixelLayer::create(u32 width, u32 height, const u8* pixels) {
    m_width = width;
    m_height = height;
    m_pixels.resize(width * height * 4);
    m_bounds = {0, 0, (f32)width, (f32)height};

    if (pixels) {
        std::memcpy(m_pixels.data(), pixels, m_pixels.size());
    } else {
        std::fill(m_pixels.begin(), m_pixels.end(), (u8)0);
    }

    m_texture = std::make_unique<Texture2D>();
    m_texture->create(width, height, m_pixels.data());
    m_dirty = false;
}

void PixelLayer::resize(u32 width, u32 height) {
    if (width == m_width && height == m_height) return;
    m_width = width;
    m_height = height;
    m_bounds = {0, 0, (f32)width, (f32)height};
    m_dirty = true;
}

void PixelLayer::uploadTexture() {
    if (m_dirty && m_texture) {
        m_texture->create(m_width, m_height, m_pixels.data());
        m_dirty = false;
    }
}

void PixelLayer::render(Renderer& renderer, const Mat4& parentTransform) {
    if (!m_visible) return;

    renderer.pushTransform(parentTransform * m_transform.matrix());
    renderer.setBlendMode(m_blendMode);

    if (m_texture) {
        uploadTexture();
        renderer.drawTexture(*m_texture, {0, 0, (f32)m_width, (f32)m_height},
                             {}, {1, 1, 1, m_opacity});
    }

    renderer.setBlendMode(BlendMode::Normal);
    renderer.popTransform();
}

void AdjustmentLayer::render(Renderer& renderer, const Mat4& parentTransform) {
    if (!m_visible) return;
    renderer.pushTransform(parentTransform * m_transform.matrix());
    renderer.popTransform();
}

void GroupLayer::addLayer(std::unique_ptr<Layer> layer) {
    m_layers.push_back(std::move(layer));
}

Layer* GroupLayer::layer(u32 index) const {
    return index < m_layers.size() ? m_layers[index].get() : nullptr;
}

std::unique_ptr<Layer> GroupLayer::removeLayer(u32 index) {
    if (index >= m_layers.size()) return nullptr;
    auto it = m_layers.begin() + index;
    auto ptr = std::move(*it);
    m_layers.erase(it);
    return ptr;
}

i32 GroupLayer::layerIndex(EntityID id) const {
    for (u32 i = 0; i < m_layers.size(); i++)
        if (m_layers[i]->id() == id) return (i32)i;
    return -1;
}

void GroupLayer::render(Renderer& renderer, const Mat4& parentTransform) {
    if (!m_visible) return;
    Mat4 localTransform = parentTransform * m_transform.matrix();
    for (auto& layer : m_layers)
        layer->render(renderer, localTransform);
}
