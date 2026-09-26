#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <any>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

using EntityID = u64;
constexpr EntityID NullID = 0;

enum class BlendMode : u8 {
    Normal,
    Multiply,
    Screen,
    Overlay,
    Darken,
    Lighten,
    ColorDodge,
    ColorBurn,
    SoftLight,
    HardLight,
    Difference,
    Exclusion,
    Hue,
    Saturation,
    Color,
    Luminosity,
    Add,
    Subtract,
    Divide
};

enum class ToolType : u8 {
    Select,
    Move,
    Brush,
    Eraser,
    Pen,
    Shape,
    Text,
    Crop,
    Eyedropper,
    Zoom,
    Hand
};

enum class LayerType : u8 {
    Pixel,
    Adjustment,
    Shape,
    Text,
    Group,
    Video,
    Audio
};

struct Extent {
    i32 x = 0, y = 0, w = 0, h = 0;

    bool contains(i32 px, i32 py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};

struct ColorRGBA {
    f32 r = 0, g = 0, b = 0, a = 1;
};

struct ColorHSLA {
    f32 h = 0, s = 0, l = 0, a = 1;
};
