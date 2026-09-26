#pragma once
#include "../core/Types.h"
#include <vector>
#include <memory>

class Effect;
class Renderer;
class Texture2D;
class Framebuffer;

class EffectPipeline {
public:
    EffectPipeline() = default;

    void addEffect(std::unique_ptr<Effect> effect);
    Effect* effect(u32 index) const;
    u32 effectCount() const { return (u32)m_effects.size(); }
    std::unique_ptr<Effect> removeEffect(u32 index);
    void moveEffect(u32 from, u32 to);
    void clear();

    void process(Renderer& renderer, Texture2D* input, Framebuffer* output);

private:
    std::vector<std::unique_ptr<Effect>> m_effects;
    std::unique_ptr<Framebuffer> m_ping, m_pong;
    bool m_usePing = false;

    void ensureFBOs(u32 w, u32 h);
};
