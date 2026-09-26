#pragma once
#include "../core/Types.h"
#include "../core/Math.h"
#include "../render/Framebuffer.h"
#include <string>
#include <vector>
#include <unordered_map>

class Texture2D;
class Renderer;

class Effect {
public:
    Effect(const std::string& name) : m_name(name) {}
    virtual ~Effect() = default;

    const std::string& name() const { return m_name; }
    bool enabled() const { return m_enabled; }
    void setEnabled(bool e) { m_enabled = e; }

    virtual void apply(Renderer& renderer, Texture2D* input, Framebuffer* output) = 0;
    virtual void renderUI() {}

protected:
    std::string m_name;
    bool m_enabled = true;
};

class BlurEffect : public Effect {
public:
    BlurEffect() : Effect("Blur") {}
    void apply(Renderer& renderer, Texture2D* input, Framebuffer* output) override;
    void renderUI() override;

    f32 radius() const { return m_radius; }
    void setRadius(f32 r) { m_radius = r; }

private:
    f32 m_radius = 5.0f;
    std::unique_ptr<Framebuffer> m_tempFB;
};

class BrightnessContrastEffect : public Effect {
public:
    BrightnessContrastEffect() : Effect("Brightness/Contrast") {}
    void apply(Renderer& renderer, Texture2D* input, Framebuffer* output) override;
    void renderUI() override;

    f32 brightness() const { return m_brightness; }
    f32 contrast() const { return m_contrast; }
    void setBrightness(f32 v) { m_brightness = v; }
    void setContrast(f32 v) { m_contrast = v; }

private:
    f32 m_brightness = 0.0f;
    f32 m_contrast = 1.0f;
};

class ColorBalanceEffect : public Effect {
public:
    ColorBalanceEffect() : Effect("Color Balance") {}
    void apply(Renderer& renderer, Texture2D* input, Framebuffer* output) override;
    void renderUI() override;

    Vec3 shadows() const { return m_shadows; }
    Vec3 midtones() const { return m_midtones; }
    Vec3 highlights() const { return m_highlights; }
    void setShadows(const Vec3& v) { m_shadows = v; }
    void setMidtones(const Vec3& v) { m_midtones = v; }
    void setHighlights(const Vec3& v) { m_highlights = v; }

private:
    Vec3 m_shadows{0,0,0};
    Vec3 m_midtones{0,0,0};
    Vec3 m_highlights{0,0,0};
};
