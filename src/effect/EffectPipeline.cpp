#include "EffectPipeline.h"
#include "Effect.h"
#include "../render/Renderer.h"
#include "../render/Texture.h"
#include "../render/Framebuffer.h"

void EffectPipeline::addEffect(std::unique_ptr<Effect> effect) {
    m_effects.push_back(std::move(effect));
}

Effect* EffectPipeline::effect(u32 index) const {
    return index < m_effects.size() ? m_effects[index].get() : nullptr;
}

std::unique_ptr<Effect> EffectPipeline::removeEffect(u32 index) {
    if (index >= m_effects.size()) return nullptr;
    auto it = m_effects.begin() + index;
    auto ptr = std::move(*it);
    m_effects.erase(it);
    return ptr;
}

void EffectPipeline::moveEffect(u32 from, u32 to) {
    if (from >= m_effects.size() || to >= m_effects.size()) return;
    auto ptr = std::move(m_effects[from]);
    m_effects.erase(m_effects.begin() + from);
    m_effects.insert(m_effects.begin() + to, std::move(ptr));
}

void EffectPipeline::clear() {
    m_effects.clear();
}

void EffectPipeline::ensureFBOs(u32 w, u32 h) {
    if (!m_ping || m_ping->width() != w || m_ping->height() != h) {
        m_ping = std::make_unique<Framebuffer>();
        m_ping->create(w, h);
        m_pong = std::make_unique<Framebuffer>();
        m_pong->create(w, h);
    }
}

void EffectPipeline::process(Renderer& renderer, Texture2D* input, Framebuffer* output) {
    if (m_effects.empty() || !input || !output) return;

    u32 w = output->width();
    u32 h = output->height();
    ensureFBOs(w, h);

    Texture2D* src = input;
    Framebuffer* dst = m_usePing ? m_ping.get() : m_pong.get();

    for (auto& effect : m_effects) {
        if (!effect->enabled()) continue;
        effect->apply(renderer, src, dst);
        src = dst->colorTexture();
        dst = (dst == m_ping.get()) ? m_pong.get() : m_ping.get();
    }

    renderer.bindFramebuffer(output);
    renderer.drawTexture(*src, {0, 0, (f32)w, (f32)h});
    renderer.unbindFramebuffer();
}
