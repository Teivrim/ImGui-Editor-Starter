#pragma once
#include "../core/Types.h"
#include <vector>

class PixelBuffer {
public:
    PixelBuffer() = default;
    PixelBuffer(u32 w, u32 h) : m_width(w), m_height(h), m_data(w * h * 4, 0) {}

    void resize(u32 w, u32 h);
    void clear(const ColorRGBA& color = {0,0,0,0});

    u8* data() { return m_data.data(); }
    const u8* data() const { return m_data.data(); }
    u32 width() const { return m_width; }
    u32 height() const { return m_height; }
    u32 size() const { return (u32)m_data.size(); }

    void setPixel(i32 x, i32 y, const ColorRGBA& color);
    ColorRGBA getPixel(i32 x, i32 y) const;

    void fill(const ColorRGBA& color);
    void blendPixel(i32 x, i32 y, const ColorRGBA& color, BlendMode mode = BlendMode::Normal);

    void blit(const PixelBuffer& src, i32 dx, i32 dy, i32 sx = 0, i32 sy = 0,
              i32 sw = -1, i32 sh = -1, BlendMode mode = BlendMode::Normal);

private:
    u32 m_width = 0, m_height = 0;
    std::vector<u8> m_data;

    static u8 clamp(f32 v) {
        return (u8)std::max(0.0f, std::min(255.0f, v * 255.0f));
    }

    static f32 blendChannel(f32 src, f32 dst, f32 a, BlendMode mode);
};
