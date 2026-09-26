#pragma once
#include "../core/Types.h"
#include "../core/Math.h"
#include <string>
#include <vector>
#include <memory>

#include "../render/Texture.h"
#include "../effect/Effect.h"

class Layer {
public:
    explicit Layer(LayerType type) : m_id(s_nextId++), m_type(type) {}
    virtual ~Layer() = default;

    EntityID id() const { return m_id; }
    LayerType type() const { return m_type; }

    void setName(const std::string& name) { m_name = name; }
    const std::string& name() const { return m_name; }

    void setVisible(bool visible) { m_visible = visible; }
    bool visible() const { return m_visible; }

    void setLocked(bool locked) { m_locked = locked; }
    bool locked() const { return m_locked; }

    void setOpacity(f32 opacity) { m_opacity = std::clamp(opacity, 0.0f, 1.0f); }
    f32 opacity() const { return m_opacity; }

    void setBlendMode(BlendMode mode) { m_blendMode = mode; }
    BlendMode blendMode() const { return m_blendMode; }

    Transform& transform() { return m_transform; }
    const Transform& transform() const { return m_transform; }

    Rect& bounds() { return m_bounds; }
    const Rect& bounds() const { return m_bounds; }

    void addEffect(std::unique_ptr<Effect> effect);
    Effect* effect(u32 index) const;
    u32 effectCount() const { return (u32)m_effects.size(); }
    void removeEffect(u32 index);

    virtual void render(class Renderer& renderer, const Mat4& parentTransform) = 0;

protected:
    EntityID m_id;
    LayerType m_type;
    std::string m_name;
    bool m_visible = true;
    bool m_locked = false;
    f32 m_opacity = 1.0f;
    BlendMode m_blendMode = BlendMode::Normal;
    Transform m_transform;
    Rect m_bounds;
    std::vector<std::unique_ptr<Effect>> m_effects;

    static EntityID s_nextId;
};

class PixelLayer : public Layer {
public:
    PixelLayer() : Layer(LayerType::Pixel) {}

    void create(u32 width, u32 height, const u8* pixels = nullptr);
    void resize(u32 width, u32 height);
    u8* pixels() { return m_pixels.data(); }
    const u8* pixels() const { return m_pixels.data(); }

    u32 width() const { return m_width; }
    u32 height() const { return m_height; }

    Texture2D* texture() { return m_texture.get(); }
    void uploadTexture();
    void markDirty() { m_dirty = true; }

    void render(Renderer& renderer, const Mat4& parentTransform) override;

private:
    u32 m_width = 0, m_height = 0;
    std::vector<u8> m_pixels;
    std::unique_ptr<Texture2D> m_texture;
    bool m_dirty = true;
};

class AdjustmentLayer : public Layer {
public:
    AdjustmentLayer(const std::string& type) : Layer(LayerType::Adjustment), m_adjustmentType(type) {}
    const std::string& adjustmentType() const { return m_adjustmentType; }
    void render(Renderer& renderer, const Mat4& parentTransform) override;
private:
    std::string m_adjustmentType;
};

class GroupLayer : public Layer {
public:
    GroupLayer() : Layer(LayerType::Group) {}

    void addLayer(std::unique_ptr<Layer> layer);
    Layer* layer(u32 index) const;
    u32 layerCount() const { return (u32)m_layers.size(); }
    std::unique_ptr<Layer> removeLayer(u32 index);
    i32 layerIndex(EntityID id) const;

    void render(Renderer& renderer, const Mat4& parentTransform) override;

private:
    std::vector<std::unique_ptr<Layer>> m_layers;
};
