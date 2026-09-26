#include "Effect.h"
#include "../render/Renderer.h"
#include "../render/Texture.h"
#include "../render/Framebuffer.h"
#include "../render/Shader.h"

#include <imgui.h>

void BlurEffect::apply(Renderer& renderer, Texture2D* input, Framebuffer* output) {
    if (!input || !output) return;
    renderer.bindFramebuffer(output);
    renderer.drawTexture(*input, {0, 0, (f32)output->width(), (f32)output->height()});
    renderer.unbindFramebuffer();
}

void BlurEffect::renderUI() {
    ImGui::SliderFloat("Radius", &m_radius, 0.0f, 100.0f, "%.1f");
}

void BrightnessContrastEffect::apply(Renderer& renderer, Texture2D* input, Framebuffer* output) {
    if (!input || !output) return;
    renderer.bindFramebuffer(output);
    renderer.drawTexture(*input, {0, 0, (f32)output->width(), (f32)output->height()});
    renderer.unbindFramebuffer();
}

void BrightnessContrastEffect::renderUI() {
    ImGui::SliderFloat("Brightness", &m_brightness, -1.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Contrast", &m_contrast, 0.0f, 3.0f, "%.2f");
}

void ColorBalanceEffect::apply(Renderer& renderer, Texture2D* input, Framebuffer* output) {
    if (!input || !output) return;
    renderer.bindFramebuffer(output);
    renderer.drawTexture(*input, {0, 0, (f32)output->width(), (f32)output->height()});
    renderer.unbindFramebuffer();
}

void ColorBalanceEffect::renderUI() {
    ImGui::Text("Shadows");
    ImGui::SliderFloat3("##shadows", &m_shadows.x, -1.0f, 1.0f, "%.2f");
    ImGui::Text("Midtones");
    ImGui::SliderFloat3("##midtones", &m_midtones.x, -1.0f, 1.0f, "%.2f");
    ImGui::Text("Highlights");
    ImGui::SliderFloat3("##highlights", &m_highlights.x, -1.0f, 1.0f, "%.2f");
}
