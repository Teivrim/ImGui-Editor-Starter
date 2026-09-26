#include "PixelBuffer.h"
#include <cstring>

void PixelBuffer::resize(u32 w, u32 h) {
    m_width = w;
    m_height = h;
    m_data.resize(w * h * 4, 0);
}

void PixelBuffer::clear(const ColorRGBA& color) {
    u8 r = (u8)(color.r * 255);
    u8 g = (u8)(color.g * 255);
    u8 b = (u8)(color.b * 255);
    u8 a = (u8)(color.a * 255);
    for (u32 i = 0; i < m_data.size(); i += 4) {
        m_data[i] = r;
        m_data[i+1] = g;
        m_data[i+2] = b;
        m_data[i+3] = a;
    }
}

void PixelBuffer::setPixel(i32 x, i32 y, const ColorRGBA& color) {
    if (x < 0 || x >= (i32)m_width || y < 0 || y >= (i32)m_height) return;
    u32 idx = ((u32)y * m_width + (u32)x) * 4;
    m_data[idx]     = clamp(color.r);
    m_data[idx + 1] = clamp(color.g);
    m_data[idx + 2] = clamp(color.b);
    m_data[idx + 3] = clamp(color.a);
}

ColorRGBA PixelBuffer::getPixel(i32 x, i32 y) const {
    if (x < 0 || x >= (i32)m_width || y < 0 || y >= (i32)m_height) return {};
    u32 idx = ((u32)y * m_width + (u32)x) * 4;
    return {m_data[idx] / 255.0f, m_data[idx+1] / 255.0f,
            m_data[idx+2] / 255.0f, m_data[idx+3] / 255.0f};
}

void PixelBuffer::fill(const ColorRGBA& color) {
    clear(color);
}

f32 PixelBuffer::blendChannel(f32 src, f32 dst, f32 a, BlendMode mode) {
    switch (mode) {
        case BlendMode::Normal:    return src * a + dst * (1 - a);
        case BlendMode::Multiply:  return src * dst;
        case BlendMode::Screen:    return 1 - (1 - src) * (1 - dst);
        case BlendMode::Add:       return src + dst;
        case BlendMode::Subtract:  return dst - src;
        default:                   return src * a + dst * (1 - a);
    }
}

void PixelBuffer::blendPixel(i32 x, i32 y, const ColorRGBA& color, BlendMode mode) {
    if (x < 0 || x >= (i32)m_width || y < 0 || y >= (i32)m_height) return;
    u32 idx = ((u32)y * m_width + (u32)x) * 4;

    f32 dr = m_data[idx] / 255.0f;
    f32 dg = m_data[idx+1] / 255.0f;
    f32 db = m_data[idx+2] / 255.0f;
    f32 da = m_data[idx+3] / 255.0f;

    f32 sr = color.r, sg = color.g, sb = color.b, sa = color.a;

    f32 outA = sa + da * (1 - sa);
    if (outA > 0) {
        m_data[idx]     = clamp((blendChannel(sr, dr, sa, mode) * sa + dr * (1 - sa)) / outA);
        m_data[idx + 1] = clamp((blendChannel(sg, dg, sa, mode) * sa + dg * (1 - sa)) / outA);
        m_data[idx + 2] = clamp((blendChannel(sb, db, sa, mode) * sa + db * (1 - sa)) / outA);
    }
    m_data[idx + 3] = clamp(outA);
}

void PixelBuffer::blit(const PixelBuffer& src, i32 dx, i32 dy, i32 sx, i32 sy,
                       i32 sw, i32 sh, BlendMode mode) {
    if (sw < 0) sw = (i32)src.m_width;
    if (sh < 0) sh = (i32)src.m_height;

    for (i32 y = 0; y < sh; y++) {
        for (i32 x = 0; x < sw; x++) {
            ColorRGBA c = src.getPixel(sx + x, sy + y);
            blendPixel(dx + x, dy + y, c, mode);
        }
    }
}
