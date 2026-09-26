#pragma once
#include "../core/Types.h"
#include "../core/Math.h"
#include <string>
#include <vector>
#include <cstdint>

class Panel {
public:
    Panel(const std::string& title, bool open = true)
        : m_title(title), m_open(open) {}
    virtual ~Panel() = default;

    const std::string& title() const { return m_title; }
    void setTitle(const std::string& t) { m_title = t; }

    bool isOpen() const { return m_open; }
    void setOpen(bool open) { m_open = open; }
    void toggle() { m_open = !m_open; }

    virtual void render() = 0;

protected:
    std::string m_title;
    bool m_open = true;
};

class CanvasPanel : public Panel {
public:
    CanvasPanel() : Panel("Canvas") {}

    Vec2 canvasPos() const { return m_canvasPos; }
    f32 zoom() const { return m_zoom; }
    void setZoom(f32 z) { m_zoom = std::clamp(z, 0.01f, 100.0f); }
    Rect viewport() const { return m_viewport; }
    Vec2 screenToCanvas(const Vec2& screen) const;

    bool showRulers() const { return m_showRulers; }
    void setShowRulers(bool v) { m_showRulers = v; }
    bool showGrid() const { return m_showGrid; }
    void setShowGrid(bool v) { m_showGrid = v; }
    f32 gridSize() const { return m_gridSize; }
    void setGridSize(f32 v) { m_gridSize = std::max(1.0f, v); }
    bool snapEnabled() const { return m_snapEnabled; }
    void setSnapEnabled(bool v) { m_snapEnabled = v; }

    void fitToScreen();

    void render() override;

private:
    Vec2 m_canvasPos{0, 0};
    f32 m_zoom = 1.0f;
    Rect m_viewport;
    bool m_showRulers = false;
    bool m_showGrid = false;
    f32 m_gridSize = 50.0f;
    bool m_snapEnabled = false;
};

class LayersPanel : public Panel {
public:
    LayersPanel() : Panel("Layers") {}
    void render() override;
    void beginRename(EntityID id) { m_renamingLayerId = id; m_renameBuf[0] = '\0'; }

private:
    EntityID m_renamingLayerId = 0;
    char m_renameBuf[128] = {};
    bool m_renameActive = false;
    char m_filterBuf[64] = {};
};

class ToolsPanel : public Panel {
public:
    ToolsPanel() : Panel("Tools") {}
    void render() override;

    ToolType activeTool() const { return m_activeTool; }
    void setActiveTool(ToolType tool) { m_activeTool = tool; s_activeTool = tool; }
    int toolHovered() const { return m_toolHovered; }

    static ToolType activeGlobal() { return s_activeTool; }

private:
    ToolType m_activeTool = ToolType::Select;
    int m_toolHovered = -1;
    static ToolType s_activeTool;
};

class TimelinePanel : public Panel {
public:
    TimelinePanel() : Panel("Timeline") {}
    void render() override;

    f64 currentTime() const { return m_currentTime; }
    void setCurrentTime(f64 t) { m_currentTime = t; }
    bool isPlaying() const { return m_playing; }
    void setPlaying(bool p) { m_playing = p; }

private:
    f64 m_currentTime = 0;
    f64 m_zoom = 1.0;
    bool m_playing = false;
};

class PropertiesPanel : public Panel {
public:
    PropertiesPanel() : Panel("Properties") {}
    void render() override;
};

class HistoryPanel : public Panel {
public:
    HistoryPanel() : Panel("History") {}
    void render() override;
};

class ColorPanel : public Panel {
public:
    ColorPanel() : Panel("Color") {}
    void render() override;

    ColorRGBA foreground() const { return m_fg; }
    ColorRGBA background() const { return m_bg; }
    void setForeground(const ColorRGBA& c) { m_fg = c; s_fg = c; }
    void setBackground(const ColorRGBA& c) { m_bg = c; s_bg = c; }
    void swapColors();

    static ColorRGBA fgColor() { return s_fg; }
    static ColorRGBA bgColor() { return s_bg; }

private:
    ColorRGBA m_fg{0, 0, 0, 1};
    ColorRGBA m_bg{1, 1, 1, 1};
    bool m_modeRGB = true;
    char m_hexBuf[8] = "000000";
    std::vector<uint32_t> m_swatches = {
        0xFF000000, 0xFFFFFFFF, 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00,
        0xFFFF00FF, 0xFF00FFFF, 0xFF808080, 0xFF800000, 0xFF008000, 0xFF000080,
        0xFF808000, 0xFF800080, 0xFF008080, 0xFFC0C0C0, 0xFF404040, 0xFFFF8000,
        0xFF00FF80, 0xFF8000FF, 0xFFFF0080, 0xFF80FF00, 0xFF0080FF, 0xFFA0522D
    };
    static ColorRGBA s_fg;
    static ColorRGBA s_bg;
};

class NavigatorPanel : public Panel {
public:
    NavigatorPanel() : Panel("Navigator") {}
    void render() override;
};

class BrushSettingsPanel : public Panel {
public:
    BrushSettingsPanel() : Panel("Brush") {}

    f32 size() const { return m_size; }
    void setSize(f32 v) { m_size = std::clamp(v, 1.0f, 2000.0f); }
    f32 hardness() const { return m_hardness; }
    f32 flow() const { return m_flow; }
    f32 spacing() const { return m_spacing; }

    void render() override;

private:
    f32 m_size = 20.0f;
    f32 m_hardness = 0.8f;
    f32 m_opacity = 1.0f;
    f32 m_flow = 1.0f;
    f32 m_spacing = 0.25f;
    int m_blendMode = 0;
    bool m_usePressure = false;
    f32 m_sizeJitter = 0.0f;
    f32 m_opacityJitter = 0.0f;
};
