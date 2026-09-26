#include "Panel.h"
#include "../core/Application.h"
#include "../ui/DockingSpace.h"
#include "../document/Document.h"
#include "../document/Composition.h"
#include "../document/Layer.h"
#include "../undo/UndoStack.h"
#include "../core/Logger.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <cstdio>
#include <cmath>
#include <cctype>
#include <algorithm>

static const char* LayerTypeIcon(LayerType type) {
    switch (type) {
        case LayerType::Pixel:      return "##";
        case LayerType::Adjustment: return " %";
        case LayerType::Shape:      return "[]";
        case LayerType::Text:       return " T";
        case LayerType::Group:      return "[]";
        default:                    return "  ";
    }
}

ToolType ToolsPanel::s_activeTool = ToolType::Select;
ColorRGBA ColorPanel::s_fg{0, 0, 0, 1};
ColorRGBA ColorPanel::s_bg{1, 1, 1, 1};

static const char* BlendModeName(BlendMode mode) {
    switch (mode) {
        case BlendMode::Normal:      return "Normal";
        case BlendMode::Multiply:    return "Multiply";
        case BlendMode::Screen:      return "Screen";
        case BlendMode::Overlay:     return "Overlay";
        case BlendMode::Darken:      return "Darken";
        case BlendMode::Lighten:     return "Lighten";
        case BlendMode::ColorDodge:  return "Color Dodge";
        case BlendMode::ColorBurn:   return "Color Burn";
        case BlendMode::SoftLight:   return "Soft Light";
        case BlendMode::HardLight:   return "Hard Light";
        case BlendMode::Difference:  return "Difference";
        case BlendMode::Exclusion:   return "Exclusion";
        case BlendMode::Hue:         return "Hue";
        case BlendMode::Saturation:  return "Saturation";
        case BlendMode::Color:       return "Color";
        case BlendMode::Luminosity:  return "Luminosity";
        case BlendMode::Add:         return "Add";
        case BlendMode::Subtract:    return "Subtract";
        case BlendMode::Divide:      return "Divide";
        default:                     return "Unknown";
    }
}

// ─── Canvas ────────────────────────────────────────────────────────────────

Vec2 CanvasPanel::screenToCanvas(const Vec2& screen) const {
    return {(screen.x - m_viewport.x) / m_zoom - m_canvasPos.x,
            (screen.y - m_viewport.y) / m_zoom - m_canvasPos.y};
}

void CanvasPanel::fitToScreen() {
    auto* doc = Application::get().activeDocument();
    if (!doc || !doc->activeComposition()) return;
    auto* comp = doc->activeComposition();
    if (m_viewport.w < 1 || m_viewport.h < 1) return;

    f32 margin = 20.0f;
    f32 availW = m_viewport.w - margin * 2;
    f32 availH = m_viewport.h - margin * 2;
    f32 compAspect = (f32)comp->width() / (f32)comp->height();
    f32 viewAspect = availW / availH;

    if (compAspect > viewAspect)
        m_zoom = availW / (f32)comp->width();
    else
        m_zoom = availH / (f32)comp->height();
    m_zoom = std::clamp(m_zoom, 0.01f, 100.0f);
    m_canvasPos = {0, 0};
}

void CanvasPanel::render() {
    if (!m_open) return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    bool visible = ImGui::Begin(m_title.c_str(), &m_open);
    ImGui::PopStyleVar();

    if (visible) {
        auto& app = Application::get();
        auto* doc = app.activeDocument();

        if (doc && doc->activeComposition()) {
            auto* comp = doc->activeComposition();
            m_viewport = {
                ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x,
                ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMin().y,
                ImGui::GetContentRegionAvail().x,
                ImGui::GetContentRegionAvail().y
            };

            ImVec2 cursor = ImGui::GetCursorScreenPos();
            f32 canvasW = m_viewport.w;
            f32 canvasH = m_viewport.h;

            ImGui::InvisibleButton("canvas", ImVec2(canvasW, canvasH),
                ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
                ImGuiButtonFlags_MouseButtonMiddle);

            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                m_canvasPos.x += ImGui::GetIO().MouseDelta.x / m_zoom;
                m_canvasPos.y += ImGui::GetIO().MouseDelta.y / m_zoom;
            }

            if (ImGui::IsWindowHovered()) {
                f32 scroll = ImGui::GetIO().MouseWheel;
                if (scroll != 0) {
                    Vec2 mouse = {(ImGui::GetIO().MousePos.x - cursor.x) / m_zoom,
                                  (ImGui::GetIO().MousePos.y - cursor.y) / m_zoom};
                    f32 oldZoom = m_zoom;
                    m_zoom *= (scroll > 0) ? 1.1f : 0.9f;
                    m_zoom = std::clamp(m_zoom, 0.01f, 100.0f);
                    m_canvasPos = m_canvasPos + mouse - mouse * (m_zoom / oldZoom);
                }
            }

            ImDrawList* draw = ImGui::GetWindowDrawList();

            // ── Rulers ──
            f32 rulerSize = m_showRulers ? 20.0f : 0.0f;

            if (m_showRulers) {
                // top ruler
                draw->AddRectFilled(cursor,
                    ImVec2(cursor.x + canvasW, cursor.y + rulerSize),
                    IM_COL32(35, 35, 40, 255));
                f32 step = 50.0f * m_zoom;
                if (step < 20) step = 20;
                if (step > 200) step = 200;
                for (f32 x = 0; x < comp->width() * m_zoom; x += step) {
                    f32 sx = cursor.x + (canvasW - comp->width() * m_zoom) / 2 + m_canvasPos.x * m_zoom + x;
                    draw->AddLine(ImVec2(sx, cursor.y), ImVec2(sx, cursor.y + rulerSize),
                        IM_COL32(80, 80, 90, 255));
                    char buf[16];
                    std::snprintf(buf, sizeof(buf), "%.0f", x / m_zoom);
                    draw->AddText(ImVec2(sx + 2, cursor.y + 2), IM_COL32(160, 160, 160, 255), buf);
                }

                // left ruler
                draw->AddRectFilled(ImVec2(cursor.x, cursor.y + rulerSize),
                    ImVec2(cursor.x + rulerSize, cursor.y + canvasH),
                    IM_COL32(35, 35, 40, 255));
                for (f32 y = 0; y < comp->height() * m_zoom; y += step) {
                    f32 sy = cursor.y + rulerSize + (canvasH - comp->height() * m_zoom) / 2 + m_canvasPos.y * m_zoom + y;
                    draw->AddLine(ImVec2(cursor.x, sy), ImVec2(cursor.x + rulerSize, sy),
                        IM_COL32(80, 80, 90, 255));
                    char buf[16];
                    std::snprintf(buf, sizeof(buf), "%.0f", y / m_zoom);
                    draw->AddText(ImVec2(cursor.x + 2, sy + 2), IM_COL32(160, 160, 160, 255), buf);
                }
            }

            f32 compW = (f32)comp->width() * m_zoom;
            f32 compH = (f32)comp->height() * m_zoom;
            Vec2 origin = {
                cursor.x + rulerSize + (canvasW - rulerSize - compW) / 2 + m_canvasPos.x * m_zoom,
                cursor.y + rulerSize + (canvasH - rulerSize - compH) / 2 + m_canvasPos.y * m_zoom
            };

            // ── Grid ──
            if (m_showGrid && m_gridSize > 0) {
                f32 gridPx = m_gridSize * m_zoom;
                if (gridPx >= 4) {
                    u32 gridCol = IM_COL32(60, 60, 70, 80);
                    for (f32 gx = 0; gx <= compW; gx += gridPx) {
                        draw->AddLine(ImVec2(origin.x + gx, origin.y),
                            ImVec2(origin.x + gx, origin.y + compH), gridCol);
                    }
                    for (f32 gy = 0; gy <= compH; gy += gridPx) {
                        draw->AddLine(ImVec2(origin.x, origin.y + gy),
                            ImVec2(origin.x + compW, origin.y + gy), gridCol);
                    }
                }
            }

            // checkerboard bg
            const i32 checkSize = 8;
            f32 cw = (f32)checkSize * m_zoom;
            if (cw < 2) cw = 2;
            for (i32 y = 0; y < (i32)(compH / cw) + 1; y++) {
                for (i32 x = 0; x < (i32)(compW / cw) + 1; x++) {
                    bool light = ((x + y) & 1) == 0;
                    draw->AddRectFilled(
                        ImVec2(origin.x + x * cw, origin.y + y * cw),
                        ImVec2(origin.x + (x + 1) * cw, origin.y + (y + 1) * cw),
                        light ? IM_COL32(55, 55, 55, 255) : IM_COL32(40, 40, 40, 255)
                    );
                }
            }

            // drop shadow
            draw->AddRect(ImVec2(origin.x - 1, origin.y - 1),
                ImVec2(origin.x + compW + 1, origin.y + compH + 1),
                IM_COL32(200, 200, 200, 80));

            // render layers
            for (u32 i = 0; i < comp->layerCount(); i++) {
                auto* layer = comp->layer(i);
                if (!layer->visible()) continue;

                auto& t = layer->transform();
                f32 lx = origin.x + t.position.x * m_zoom;
                f32 ly = origin.y + t.position.y * m_zoom;
                f32 lw = (f32)comp->width() * m_zoom * t.scale.x;
                f32 lh = (f32)comp->height() * m_zoom * t.scale.y;

                draw->AddRectFilled(ImVec2(lx, ly), ImVec2(lx + lw, ly + lh),
                    IM_COL32(80 + i * 30, 60, 120, 180));
                draw->AddRect(ImVec2(lx, ly), ImVec2(lx + lw, ly + lh),
                    IM_COL32(150, 200, 255, 200));

                char buf[64];
                std::snprintf(buf, sizeof(buf), "%s [%.0f%%]", layer->name().c_str(), layer->opacity() * 100);
                draw->AddText(ImVec2(lx + 4, ly + 4), IM_COL32_WHITE, buf);
            }

            // ── Transform handles (Move tool + selected layer) ──
            if (ToolsPanel::activeGlobal() == ToolType::Move || ToolsPanel::activeGlobal() == ToolType::Select) {
                auto* sel = comp->selectedLayer();
                if (sel && sel->visible()) {
                    auto& t = sel->transform();
                    f32 hx = origin.x + t.position.x * m_zoom;
                    f32 hy = origin.y + t.position.y * m_zoom;
                    f32 hw = compW * t.scale.x;
                    f32 hh = compH * t.scale.y;

                    // selection rect
                    draw->AddRect(ImVec2(hx, hy), ImVec2(hx + hw, hy + hh),
                        IM_COL32(255, 255, 0, 180), 0, 0, 2.0f / m_zoom);

                    // corner handles
                    f32 hs = 6.0f;
                    ImVec2 corners[4] = {
                        ImVec2(hx, hy), ImVec2(hx + hw, hy),
                        ImVec2(hx, hy + hh), ImVec2(hx + hw, hy + hh)
                    };
                    for (int ci = 0; ci < 4; ci++) {
                        draw->AddRectFilled(
                            ImVec2(corners[ci].x - hs, corners[ci].y - hs),
                            ImVec2(corners[ci].x + hs, corners[ci].y + hs),
                            IM_COL32_WHITE);
                        draw->AddRect(
                            ImVec2(corners[ci].x - hs, corners[ci].y - hs),
                            ImVec2(corners[ci].x + hs, corners[ci].y + hs),
                            IM_COL32(0, 0, 0, 200));
                    }

                    // center move handle
                    ImVec2 center(hx + hw / 2, hy + hh / 2);
                    draw->AddCircleFilled(center, 4, IM_COL32(255, 255, 255, 200));
                    draw->AddCircle(center, 4, IM_COL32(0, 0, 0, 200));
                }
            }

            // crosshair center
            ImVec2 center(origin.x + compW / 2, origin.y + compH / 2);
            draw->AddLine(ImVec2(center.x - 10, center.y), ImVec2(center.x + 10, center.y),
                IM_COL32(255, 0, 0, 100));
            draw->AddLine(ImVec2(center.x, center.y - 10), ImVec2(center.x, center.y + 10),
                IM_COL32(255, 0, 0, 100));

            // zoom badge
            char zoomText[32];
            std::snprintf(zoomText, sizeof(zoomText), "%.0f%%", m_zoom * 100);
            ImVec2 zoomPos(ImGui::GetWindowPos().x + 8, ImGui::GetWindowPos().y + 8);
            draw->AddRectFilled(ImVec2(zoomPos.x - 2, zoomPos.y - 2),
                ImVec2(zoomPos.x + 56, zoomPos.y + 20), IM_COL32(0, 0, 0, 160));
            draw->AddText(zoomPos, IM_COL32_WHITE, zoomText);

            // ── Canvas right-click context menu ──
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                ImGui::OpenPopup("##CanvasContext");

            if (ImGui::BeginPopup("##CanvasContext")) {
                auto* dock = Application::get().dockingSpace();
                auto* tools = dock ? dock->findPanel<ToolsPanel>() : nullptr;

                if (ImGui::MenuItem("Select (V)"))  { if (tools) tools->setActiveTool(ToolType::Select); }
                if (ImGui::MenuItem("Move (M)"))    { if (tools) tools->setActiveTool(ToolType::Move); }
                if (ImGui::MenuItem("Brush (B)"))   { if (tools) tools->setActiveTool(ToolType::Brush); }
                if (ImGui::MenuItem("Eraser (E)"))  { if (tools) tools->setActiveTool(ToolType::Eraser); }
                if (ImGui::MenuItem("Eyedropper (I)")) { if (tools) tools->setActiveTool(ToolType::Eyedropper); }
                ImGui::Separator();
                if (ImGui::MenuItem("Zoom In"))     { m_zoom = std::clamp(m_zoom * 1.2f, 0.01f, 100.0f); }
                if (ImGui::MenuItem("Zoom Out"))    { m_zoom = std::clamp(m_zoom / 1.2f, 0.01f, 100.0f); }
                if (ImGui::MenuItem("Fit to Screen")) { fitToScreen(); }
                ImGui::Separator();
                bool grid = m_showGrid;
                if (ImGui::MenuItem("Show Grid", nullptr, &grid)) m_showGrid = grid;
                bool rulers = m_showRulers;
                if (ImGui::MenuItem("Show Rulers", nullptr, &rulers)) m_showRulers = rulers;
                bool snap = m_snapEnabled;
                if (ImGui::MenuItem("Snap", nullptr, &snap)) m_snapEnabled = snap;
                ImGui::EndPopup();
            }

            // ── Brush / Eyedropper ──
            auto& io = ImGui::GetIO();
            ToolType activeTool = ToolsPanel::activeGlobal();

            // brush preview circle
            if (activeTool == ToolType::Brush && ImGui::IsItemHovered()) {
                BrushSettingsPanel* brushPanel = nullptr;
                auto* dock = Application::get().dockingSpace();
                if (dock) brushPanel = dock->findPanel<BrushSettingsPanel>();
                f32 bs = brushPanel ? brushPanel->size() : 20.0f;
                f32 previewR = bs * m_zoom / 2;
                ImVec2 mousePos = io.MousePos;
                draw->AddCircle(mousePos, previewR, IM_COL32(200, 200, 200, 160), 0, 1.5f);
                draw->AddCircle(mousePos, previewR * 0.5f, IM_COL32(200, 200, 200, 80), 0, 1.0f);
                draw->AddLine(ImVec2(mousePos.x - previewR, mousePos.y),
                    ImVec2(mousePos.x + previewR, mousePos.y), IM_COL32(200, 200, 200, 60));
                draw->AddLine(ImVec2(mousePos.x, mousePos.y - previewR),
                    ImVec2(mousePos.x, mousePos.y + previewR), IM_COL32(200, 200, 200, 60));
            }

            if (activeTool == ToolType::Brush && ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                BrushSettingsPanel* brushPanel = nullptr;
                auto* dock = Application::get().dockingSpace();
                if (dock) brushPanel = dock->findPanel<BrushSettingsPanel>();
                f32 brushSize = brushPanel ? brushPanel->size() : 20.0f;
                f32 brushHardness = brushPanel ? brushPanel->hardness() : 0.8f;
                f32 brushFlow = brushPanel ? brushPanel->flow() : 1.0f;

                f32 mx = (io.MousePos.x - origin.x) / m_zoom;
                f32 my = (io.MousePos.y - origin.y) / m_zoom;

                for (i32 li = (i32)comp->layerCount() - 1; li >= 0; li--) {
                    auto* layer = comp->layer((u32)li);
                    if (!layer->visible() || layer->locked()) continue;
                    auto* pl = dynamic_cast<PixelLayer*>(layer);
                    if (!pl) continue;

                    f32 lx = mx - layer->transform().position.x;
                    f32 ly = my - layer->transform().position.y;

                    i32 px = (i32)(lx / layer->transform().scale.x);
                    i32 py = (i32)(ly / layer->transform().scale.y);

                    f32 half = brushSize / 2.0f;
                    i32 minx = std::max(0, (i32)(px - half));
                    i32 miny = std::max(0, (i32)(py - half));
                    i32 maxx = std::min((i32)pl->width() - 1, (i32)(px + half));
                    i32 maxy = std::min((i32)pl->height() - 1, (i32)(py + half));

                    auto* pixels = pl->pixels();
                    auto fg = ColorPanel::fgColor();
                    u8 r = (u8)(fg.r * 255);
                    u8 g = (u8)(fg.g * 255);
                    u8 b = (u8)(fg.b * 255);
                    u8 a = (u8)(fg.a * 255 * brushFlow);

                    for (i32 dy = miny; dy <= maxy; dy++) {
                        for (i32 dx = minx; dx <= maxx; dx++) {
                            f32 dist = std::sqrt((f32)((dx - px) * (dx - px) + (dy - py) * (dy - py)));
                            if (dist > half) continue;
                            u32 idx = ((u32)dy * pl->width() + (u32)dx) * 4;
                            f32 influence = 1.0f - dist / half;
                            influence = std::pow(influence, 1.0f + (1.0f - brushHardness) * 4.0f);
                            u8 srcR = pixels[idx], srcG = pixels[idx+1], srcB = pixels[idx+2], srcA = pixels[idx+3];
                            pixels[idx]     = (u8)(srcR * (1 - influence) + r * influence);
                            pixels[idx + 1] = (u8)(srcG * (1 - influence) + g * influence);
                            pixels[idx + 2] = (u8)(srcB * (1 - influence) + b * influence);
                            pixels[idx + 3] = (u8)(srcA * (1 - influence) + a * influence);
                        }
                    }

                    pl->markDirty();
                    pl->uploadTexture();
                    if (auto* d = app.activeDocument()) d->setModified(true);
                    break;
                }
            }

            // ── Eyedropper ──
            if (activeTool == ToolType::Eyedropper && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                f32 mx = (io.MousePos.x - origin.x) / m_zoom;
                f32 my = (io.MousePos.y - origin.y) / m_zoom;
                for (i32 li = (i32)comp->layerCount() - 1; li >= 0; li--) {
                    auto* layer = comp->layer((u32)li);
                    if (!layer->visible()) continue;
                    auto* pl = dynamic_cast<PixelLayer*>(layer);
                    if (!pl) continue;

                    i32 px = (i32)(mx - layer->transform().position.x);
                    i32 py = (i32)(my - layer->transform().position.y);
                    if (px >= 0 && px < (i32)pl->width() && py >= 0 && py < (i32)pl->height()) {
                        u32 idx = ((u32)py * pl->width() + (u32)px) * 4;
                        auto* pixels = pl->pixels();
                        ColorRGBA picked = {
                            pixels[idx] / 255.0f, pixels[idx + 1] / 255.0f,
                            pixels[idx + 2] / 255.0f, pixels[idx + 3] / 255.0f
                        };
                        auto* dock = Application::get().dockingSpace();
                        auto* colorPanel = dock ? dock->findPanel<ColorPanel>() : nullptr;
                        if (colorPanel) colorPanel->setForeground(picked);
                        Logger::instance().info("Picked color: " +
                            std::to_string((int)pixels[idx]) + "," +
                            std::to_string((int)pixels[idx + 1]) + "," +
                            std::to_string((int)pixels[idx + 2]));
                    }
                    break;
                }
            }

            // ── Move tool: drag selected layer ──
            if ((activeTool == ToolType::Move || activeTool == ToolType::Select) && ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                auto* sel = comp->selectedLayer();
                if (sel && !sel->locked()) {
                    auto& t = sel->transform();
                    t.position.x += io.MouseDelta.x / m_zoom;
                    t.position.y += io.MouseDelta.y / m_zoom;
                    if (auto* d = app.activeDocument()) d->setModified(true);
                }
            }
        } else {
            ImGui::SetCursorPos(ImVec2(
                ImGui::GetWindowWidth() / 2 - 80,
                ImGui::GetWindowHeight() / 2 - 10
            ));
            ImGui::TextUnformatted("No document open");
        }
    }
    ImGui::End();
}

// ─── Layers ────────────────────────────────────────────────────────────────

void LayersPanel::render() {
    if (!m_open) return;
    if (!ImGui::Begin(m_title.c_str(), &m_open)) { ImGui::End(); return; }

    auto* doc = Application::get().activeDocument();
    if (doc && doc->activeComposition()) {
        auto* comp = doc->activeComposition();

        // add layer button
        if (ImGui::Button("+", ImVec2(24, 24))) {
            auto layer = std::make_unique<PixelLayer>();
            layer->setName("Layer " + std::to_string(comp->layerCount() + 1));
            layer->create(comp->width(), comp->height());
            comp->addLayer(std::move(layer));
            if (auto* d = Application::get().activeDocument()) d->setModified(true);
            Logger::instance().info("Added new layer");
        }
        ImGui::SameLine();
        if (ImGui::Button("-", ImVec2(24, 24))) {
            if (comp->layerCount() > 0) {
                comp->removeLayer(comp->layerCount() - 1);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("F", ImVec2(24, 24))) {
            auto group = std::make_unique<GroupLayer>();
            group->setName("Group " + std::to_string(comp->layerCount() + 1));
            comp->addLayer(std::move(group));
            if (auto* d = Application::get().activeDocument()) d->setModified(true);
            Logger::instance().info("Added group layer");
        }
        ImGui::SameLine();
        ImGui::Text("  %u", comp->layerCount());

        ImGui::Separator();

        // search / filter
        ImGui::SetNextItemWidth(-1);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
        ImGui::InputTextWithHint("##layer_filter", "Filter layers...", m_filterBuf, sizeof(m_filterBuf));
        ImGui::PopStyleVar();

        // layer list
        ImGui::BeginChild("LayersList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 3), false);

        u32 selected = comp->selectedLayerIndex();
        std::string filter = m_filterBuf;
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

        for (i32 i = (i32)comp->layerCount() - 1; i >= 0; i--) {
            auto* layer = comp->layer((u32)i);

            // apply filter
            if (!filter.empty()) {
                std::string lower = layer->name();
                std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                if (lower.find(filter) == std::string::npos)
                    continue;
            }

            ImGui::PushID((int)layer->id());

            bool isSelected = ((u32)i == selected);

            // highlight row
            if (isSelected) {
                ImVec2 p0 = ImGui::GetCursorScreenPos();
                ImVec2 p1 = ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y + ImGui::GetTextLineHeightWithSpacing() + 4);
                ImGui::GetWindowDrawList()->AddRectFilled(p0, p1, IM_COL32(50, 80, 140, 120));
            }

            // visibility eye
            bool visible = layer->visible();
            if (ImGui::Checkbox("##v", &visible))
                layer->setVisible(visible);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            ImGui::SameLine();

            // lock toggle
            bool locked = layer->locked();
            if (ImGui::Checkbox("##l", &locked)) {
                layer->setLocked(locked);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }
            ImGui::SameLine();

            // layer thumbnail placeholder
            ImGui::ColorButton("thumb",
                isSelected ? ImVec4(0.4f, 0.5f, 0.7f, 1.0f) : ImVec4(0.3f, 0.4f, 0.6f, 1.0f),
                ImGuiColorEditFlags_NoTooltip, ImVec2(20, 20));
            ImGui::SameLine();

            // layer name + type icon
            ImGui::Text("%s ", LayerTypeIcon(layer->type()));
            ImGui::SameLine();

            if (locked) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

            // rename on double-click or via active rename state
            bool isRenaming = (layer->id() == m_renamingLayerId);
            if (isRenaming) {
                ImGui::SetNextItemWidth(120);
                if (m_renameBuf[0] == '\0')
                    std::snprintf(m_renameBuf, sizeof(m_renameBuf), "%s", layer->name().c_str());
                ImGui::SetKeyboardFocusHere();
                if (ImGui::InputText("##rename", m_renameBuf, sizeof(m_renameBuf),
                        ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
                    layer->setName(m_renameBuf);
                    if (auto* d = Application::get().activeDocument()) d->setModified(true);
                    m_renamingLayerId = 0;
                    m_renameBuf[0] = '\0';
                }
                if (!ImGui::IsItemActive()) {
                    if (m_renameBuf[0] != '\0') {
                        layer->setName(m_renameBuf);
                        if (auto* d = Application::get().activeDocument()) d->setModified(true);
                    }
                    m_renamingLayerId = 0;
                    m_renameBuf[0] = '\0';
                }
            } else {
                char label[128];
                std::snprintf(label, sizeof(label), "%s##layer_%d", layer->name().c_str(), (int)layer->id());
                if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        m_renamingLayerId = layer->id();
                        m_renameBuf[0] = '\0';
                    }
                    comp->setSelectedLayer((u32)i);
                }
            }

            if (locked) ImGui::PopStyleColor();

            // drag source (only if not locked)
            if (!locked && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                ImGui::SetDragDropPayload("LAYER_DRAG", &i, sizeof(i32));
                ImGui::Text("%s", layer->name().c_str());
                ImGui::EndDragDropSource();
            }

            // drop target
            if (ImGui::BeginDragDropTarget()) {
                if (const auto* payload = ImGui::AcceptDragDropPayload("LAYER_DRAG")) {
                    i32 srcIdx = *(const i32*)payload->Data;
                    if (srcIdx >= 0 && srcIdx < (i32)comp->layerCount()) {
                        auto srcLayer = comp->removeLayer((u32)srcIdx);
                        if (srcLayer) {
                            comp->addLayer(std::move(srcLayer));
                            if (auto* d = Application::get().activeDocument()) d->setModified(true);
                            Logger::instance().info("Reordered layer");
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // right-click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Select")) { comp->setSelectedLayer((u32)i); }
                if (ImGui::MenuItem("Rename")) { m_renamingLayerId = layer->id(); m_renameBuf[0] = '\0'; }
                if (ImGui::MenuItem("Duplicate", "Ctrl+J")) {
                    auto dup = std::make_unique<PixelLayer>();
                    dup->setName(layer->name() + " copy");
                    dup->setVisible(layer->visible());
                    dup->setOpacity(layer->opacity());
                    dup->setBlendMode(layer->blendMode());
                    dup->transform() = layer->transform();
                    if (auto* pl = dynamic_cast<PixelLayer*>(layer)) {
                        dup->create(pl->width(), pl->height(), pl->pixels());
                        dup->uploadTexture();
                    }
                    comp->addLayer(std::move(dup));
                    if (auto* d = Application::get().activeDocument()) d->setModified(true);
                }
                if (ImGui::MenuItem("Delete", "Del")) {
                    comp->removeLayer((u32)i);
                    if (auto* d = Application::get().activeDocument()) d->setModified(true);
                    ImGui::EndPopup();
                    ImGui::PopID();
                    break;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Merge Down", "Ctrl+E")) {}
                if (ImGui::MenuItem("Merge Visible")) {}
                if (ImGui::MenuItem("Flatten Image")) {}
                ImGui::Separator();
                if (ImGui::BeginMenu("Blend Mode")) {
                    auto currentMode = layer->blendMode();
                    for (int m = 0; m <= (int)BlendMode::Divide; m++) {
                        bool s = (int)currentMode == m;
                        if (ImGui::MenuItem(BlendModeName((BlendMode)m), nullptr, s))
                            layer->setBlendMode((BlendMode)m);
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Lock")) {
                    bool lk = layer->locked();
                    if (ImGui::MenuItem("Lock All", nullptr, &lk)) layer->setLocked(lk);
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        ImGui::EndChild();

        // opacity + fill sliders at bottom
        ImGui::Separator();
        if (comp->layerCount() > 0) {
            auto* layer = comp->selectedLayer();
            if (!layer) layer = comp->layer(0);
            f32 op = layer->opacity();
            if (ImGui::SliderFloat("Opacity", &op, 0, 1, "%.0f%%")) {
                layer->setOpacity(op);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }
            f32 fl = 1.0f;
            if (ImGui::SliderFloat("Fill", &fl, 0, 1, "%.0f%%")) {
                // fill is pixel opacity only (separate from layer effects)
            }
        }
    }

    ImGui::End();
}

// ─── Tools ─────────────────────────────────────────────────────────────────

void ToolsPanel::render() {
    if (!m_open) return;
    if (!ImGui::Begin(m_title.c_str(), &m_open)) { ImGui::End(); return; }

    const char* toolIcons[] = {
        "V", "M", "B", "E", "P", "U",
        "T", "C", "I", "Z", "H"
    };
    const char* toolNames[] = {
        "Select (V)", "Move (M)", "Brush (B)", "Eraser (E)", "Pen (P)", "Shape (U)",
        "Text (T)", "Crop (C)", "Eyedropper (I)", "Zoom (Z)", "Hand (H)"
    };

    int current = (int)m_activeTool;

    for (int i = 0; i < IM_ARRAYSIZE(toolIcons); i++) {
        if (i > 0 && i % 2 == 0)
            ImGui::Spacing();

        bool isActive = (current == i);
        ImVec4 btnColor = isActive ? ImVec4(0.3f, 0.5f, 0.8f, 1.0f) : ImVec4(0.15f, 0.15f, 0.15f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.9f, 1.0f));

        if (ImGui::Button(toolIcons[i], ImVec2(32, 28))) {
            m_activeTool = (ToolType)i;
            s_activeTool = (ToolType)i;
        }

        ImGui::PopStyleColor(2);

        if (ImGui::IsItemHovered()) {
            m_toolHovered = i;
        }

        if (i % 2 == 0)
            ImGui::SameLine();
    }

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("%s", toolNames[(int)m_activeTool]);
    ImGui::Separator();

    // tool-specific options - only basic ones, BrushSettingsPanel handles brush
    switch (m_activeTool) {
        case ToolType::Eraser: {
            static f32 eraserSize = 20;
            ImGui::SliderFloat("Size", &eraserSize, 1, 500, "%.0f");
            break;
        }
        case ToolType::Pen: {
            static f32 penWidth = 2;
            static bool penSmooth = true;
            ImGui::SliderFloat("Width", &penWidth, 1, 50, "%.0f");
            ImGui::Checkbox("Smooth", &penSmooth);
            break;
        }
        case ToolType::Shape: {
            static int shapeType = 0;
            const char* shapes[] = {"Rectangle", "Ellipse", "Line", "Polygon"};
            ImGui::Combo("Type", &shapeType, shapes, IM_ARRAYSIZE(shapes));
            static bool filled = true;
            ImGui::Checkbox("Filled", &filled);
            break;
        }
        case ToolType::Text: {
            static char textBuf[256] = "";
            static f32 fontSize = 24;
            ImGui::InputText("Text", textBuf, sizeof(textBuf));
            ImGui::SliderFloat("Size", &fontSize, 8, 200, "%.0f");
            break;
        }
        default:
            break;
    }

    ImGui::End();
}

// ─── Timeline ──────────────────────────────────────────────────────────────

void TimelinePanel::render() {
    if (!m_open) return;
    if (!ImGui::Begin(m_title.c_str(), &m_open)) { ImGui::End(); return; }

    auto* doc = Application::get().activeDocument();
    if (doc && doc->activeComposition()) {
        auto* comp = doc->activeComposition();
        f64 duration = comp->duration();
        f64 frameRate = comp->frameRate();
        f32 availWidth = ImGui::GetContentRegionAvail().x;

        // ── transport controls ──
        {
            bool playing = m_playing;

            if (ImGui::Button("|-<", ImVec2(36, 24))) {
                m_currentTime = 0;
                for (u32 i = 0; i < comp->layerCount(); i++)
                    comp->layer(i)->transform().position = {0, 0};
            }
            ImGui::SameLine();
            if (ImGui::Button("<", ImVec2(28, 24))) {
                m_currentTime = std::max(0.0, m_currentTime - 1.0 / frameRate);
            }
            ImGui::SameLine();

            if (ImGui::Button(playing ? "||" : ">", ImVec2(32, 24))) {
                m_playing = !m_playing;
            }
            ImGui::SameLine();

            if (ImGui::Button(">", ImVec2(28, 24))) {
                m_currentTime = std::min(duration, m_currentTime + 1.0 / frameRate);
            }
            ImGui::SameLine();
            if (ImGui::Button(">|", ImVec2(36, 24))) { m_currentTime = duration; }

            // frame counter
            ImGui::SameLine(ImGui::GetWindowWidth() - 160);
            i32 currentFrame = (i32)(m_currentTime * frameRate + 0.5);
            i32 totalFrames = (i32)(duration * frameRate + 0.5);
            ImGui::Text("Frame %d / %d", currentFrame, totalFrames);
        }

        // ── time slider ──
        f64 timeMin = 0.0;
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderScalarN("##timeslider", ImGuiDataType_Double, &m_currentTime, 1, &timeMin, &duration, "%.2f");
        m_currentTime = std::clamp(m_currentTime, 0.0, duration);

        ImGui::Separator();

        // ── time ruler ──
        ImGui::BeginChild("TimelineTracks", ImVec2(0, 0), false);

        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 rulerPos = ImGui::GetCursorScreenPos();
        f32 rulerH = 24;
        f32 trackH = 28;
        f32 labelW = 80;
        f32 timeScale = (availWidth - labelW) / (f32)duration;

        // ruler bg
        draw->AddRectFilled(rulerPos, ImVec2(rulerPos.x + availWidth, rulerPos.y + rulerH),
            IM_COL32(30, 30, 35, 255));

        // second markers
        for (i32 s = 0; s <= (i32)duration; s++) {
            f32 x = rulerPos.x + labelW + s * timeScale;
            draw->AddLine(ImVec2(x, rulerPos.y), ImVec2(x, rulerPos.y + rulerH),
                IM_COL32(80, 80, 90, 255));
            char label[16];
            std::snprintf(label, sizeof(label), "%ds", s);
            draw->AddText(ImVec2(x + 2, rulerPos.y + 2), IM_COL32(180, 180, 180, 255), label);
        }

        // playhead
        f32 phx = rulerPos.x + labelW + (f32)m_currentTime * timeScale;
        draw->AddLine(ImVec2(phx, rulerPos.y), ImVec2(phx, rulerPos.y + rulerH + trackH * comp->layerCount() + 10),
            IM_COL32(255, 50, 50, 220));

        // playhead drag
        ImGui::SetCursorScreenPos(ImVec2(rulerPos.x + labelW, rulerPos.y));
        ImGui::InvisibleButton("##playhead_drag", ImVec2(availWidth - labelW, rulerH));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            f32 relX = ImGui::GetIO().MousePos.x - (rulerPos.x + labelW);
            m_currentTime = std::clamp((f64)(relX / timeScale), 0.0, duration);
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + rulerH);

        // ── layer tracks ──
        for (u32 i = 0; i < comp->layerCount(); i++) {
            auto* layer = comp->layer(i);

            ImVec2 trackPos = ImGui::GetCursorScreenPos();

            // track bg
            draw->AddRectFilled(trackPos,
                ImVec2(trackPos.x + availWidth, trackPos.y + trackH),
                (i % 2 == 0) ? IM_COL32(25, 25, 30, 255) : IM_COL32(20, 20, 25, 255));

            // highlight selected track
            if (i == comp->selectedLayerIndex()) {
                draw->AddRectFilled(trackPos,
                    ImVec2(trackPos.x + availWidth, trackPos.y + trackH),
                    IM_COL32(50, 80, 140, 60));
            }

            // layer name label
            draw->AddText(ImVec2(trackPos.x + 4, trackPos.y + 5), IM_COL32(200, 200, 200, 255),
                layer->name().c_str());

            // visibility indicator
            if (!layer->visible()) {
                draw->AddText(ImVec2(trackPos.x + 4, trackPos.y + 5), IM_COL32(100, 100, 100, 255), "[H]");
            }

            // layer bar
            f32 barX = trackPos.x + labelW;
            f32 barW = availWidth - labelW;
            draw->AddRectFilled(ImVec2(barX, trackPos.y + 4),
                ImVec2(barX + barW, trackPos.y + trackH - 4),
                IM_COL32(60 + i * 20, 80, 120, 200));
            draw->AddRect(ImVec2(barX, trackPos.y + 4),
                ImVec2(barX + barW, trackPos.y + trackH - 4),
                IM_COL32(100, 150, 200, 180));

            // keyframe dots at every second
            for (i32 s = 0; s <= (i32)comp->duration(); s++) {
                f32 kx = barX + s * timeScale;
                draw->AddCircleFilled(ImVec2(kx, trackPos.y + trackH / 2), 3,
                    IM_COL32(255, 220, 50, 200));
            }

            // click to select track
            ImGui::SetCursorScreenPos(trackPos);
            ImGui::InvisibleButton("##track", ImVec2(availWidth, trackH));
            if (ImGui::IsItemClicked()) {
                comp->setSelectedLayer(i);
            }

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + trackH);
        }

        ImGui::EndChild();
    }

    ImGui::End();
}

// ─── Properties ────────────────────────────────────────────────────────────

void PropertiesPanel::render() {
    if (!m_open) return;
    if (!ImGui::Begin(m_title.c_str(), &m_open)) { ImGui::End(); return; }

    auto* doc = Application::get().activeDocument();
    if (!doc) { ImGui::TextUnformatted("No document"); ImGui::End(); return; }

    auto* comp = doc->activeComposition();
    if (!comp) { ImGui::TextUnformatted("No active composition"); ImGui::End(); return; }

    // ── Comp properties ──
    if (ImGui::CollapsingHeader("Composition", ImGuiTreeNodeFlags_DefaultOpen)) {
        int w = (int)comp->width(), h = (int)comp->height();
        ImGui::SetNextItemWidth(80); ImGui::InputInt("Width", &w, 0, 0);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80); ImGui::InputInt("Height", &h, 0, 0);

        f32 aspect = (f32)w / (f32)h;
        ImGui::Text("Aspect: %.2f:1", aspect);

        f32 fr = (f32)comp->frameRate();
        ImGui::SetNextItemWidth(100);
        if (ImGui::InputFloat("FPS", &fr, 1, 10, "%.1f"))
            comp->setFrameRate(fr);

        f64 dur = comp->duration();
        ImGui::SetNextItemWidth(100);
        if (ImGui::InputDouble("Duration (s)", &dur, 1, 10, "%.1f"))
            comp->setDuration(dur);

        i32 totalFrames = (i32)(dur * fr + 0.5);
        ImGui::Text("Frames: %d", totalFrames);
    }

    // ── Selected layer properties ──
    auto* selLayer = comp->selectedLayer();
    if (ImGui::CollapsingHeader("Layer", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (!selLayer) {
            ImGui::TextUnformatted("No layers");
        } else {
            auto* layer = selLayer;

            char nameBuf[128];
            std::snprintf(nameBuf, sizeof(nameBuf), "%s", layer->name().c_str());
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
                layer->setName(nameBuf);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }

            auto& t = layer->transform();

            ImGui::SeparatorText("Transform");

            f32 pos[2] = {t.position.x, t.position.y};
            if (ImGui::DragFloat2("Position", pos, 1.0f, -10000, 10000, "%.1f")) {
                t.position = {pos[0], pos[1]};
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }

            f32 scl[2] = {t.scale.x, t.scale.y};
            if (ImGui::DragFloat2("Scale", scl, 0.01f, -10, 10, "%.2f")) {
                t.scale = {scl[0], scl[1]};
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }

            f32 rot = t.rotation;
            if (ImGui::SliderFloat("Rotation", &rot, -360, 360, "%.1f")) {
                t.rotation = rot;
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }

            f32 op = layer->opacity();
            if (ImGui::SliderFloat("Opacity", &op, 0, 1, "%.0f%%")) {
                layer->setOpacity(op);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }

            ImGui::SeparatorText("Blending");

            int blend = (int)layer->blendMode();
            if (ImGui::Combo("Mode", &blend, BlendModeName(BlendMode::Normal))) {
                layer->setBlendMode((BlendMode)blend);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }

            ImGui::SeparatorText("Info");
            ImGui::Text("ID: %llu", (unsigned long long)layer->id());
            ImGui::Text("Type: %d", (int)layer->type());
            ImGui::Text("Visible: %s", layer->visible() ? "yes" : "no");
            ImGui::Text("Locked: %s", layer->locked() ? "yes" : "no");
            ImGui::Text("Effects: %u", layer->effectCount());
        }
    }

    // ── Effects ──
    if (selLayer && ImGui::CollapsingHeader("Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (u32 ei = 0; ei < selLayer->effectCount(); ei++) {
            auto* effect = selLayer->effect(ei);
            ImGui::PushID(ei);

            bool enabled = effect->enabled();
            if (ImGui::Checkbox("##en", &enabled)) {
                effect->setEnabled(enabled);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
            }
            ImGui::SameLine();

            ImGui::Text("%s", effect->name().c_str());

            ImGui::SameLine(ImGui::GetWindowWidth() - 80);
            if (ImGui::SmallButton("Params")) {
                Logger::instance().info("Effect params: " + effect->name());
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("X")) {
                selLayer->removeEffect(ei);
                if (auto* d = Application::get().activeDocument()) d->setModified(true);
                ImGui::PopID();
                break;
            }

            ImGui::PopID();
        }

        if (ImGui::Button("+ Add Effect", ImVec2(-1, 24))) {
            ImGui::OpenPopup("AddEffectPopup");
        }

        if (ImGui::BeginPopup("AddEffectPopup")) {
            const char* effects[] = {"Brightness/Contrast", "Hue/Saturation", "Levels", "Blur", "Sharpen", "Color Balance"};
            for (int ei = 0; ei < IM_ARRAYSIZE(effects); ei++) {
                if (ImGui::MenuItem(effects[ei])) {
                    Logger::instance().info("Add effect: " + std::string(effects[ei]));
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

// ─── History ───────────────────────────────────────────────────────────────

void HistoryPanel::render() {
    if (!m_open) return;
    if (!ImGui::Begin(m_title.c_str(), &m_open)) { ImGui::End(); return; }

    auto& stack = Application::get().undoStack();

    ImGui::BeginDisabled(!stack.canUndo());
    if (ImGui::Button("Undo", ImVec2(-1, 26))) stack.undo();
    ImGui::EndDisabled();

    ImGui::BeginDisabled(!stack.canRedo());
    if (ImGui::Button("Redo", ImVec2(-1, 26))) stack.redo();
    ImGui::EndDisabled();

    ImGui::SeparatorText("History");

    ImGui::BeginChild("HistoryList", ImVec2(0, 0), false);

    if (stack.commandCount() == 0) {
        ImGui::TextDisabled("No commands yet");
    }

    // command list, newest at bottom, click to undo/redo to that point
    u32 count = stack.commandCount();
    i32 current = stack.currentIndex();
    for (i32 ci = (i32)count - 1; ci >= 0; ci--) {
        std::string name = stack.commandName((u32)ci);
        if (name.empty()) continue;

        bool isCurrent = (ci == current);
        bool isApplied = (ci < current);

        ImGui::PushID(ci);
        ImGui::PushStyleColor(ImGuiCol_Text,
            isCurrent ? ImVec4(0.4f, 0.7f, 1.0f, 1.0f) :
            isApplied ? ImVec4(0.8f, 0.8f, 0.8f, 1.0f) :
                        ImVec4(0.45f, 0.45f, 0.45f, 1.0f));

        char label[160];
        std::snprintf(label, sizeof(label), "%s##hist_%d", name.c_str(), ci);
        if (ImGui::Selectable(label, isCurrent)) {
            stack.undoTo(ci);
        }
        if (isCurrent) ImGui::SetItemDefaultFocus();

        ImGui::PopStyleColor();
        ImGui::PopID();
    }

    ImGui::Separator();
    if (ImGui::Button("Clear History", ImVec2(-1, 24)))
        stack.clear();

    ImGui::EndChild();
    ImGui::End();
}

// ─── Color ─────────────────────────────────────────────────────────────────

void ColorPanel::render() {
    if (!m_open) return;
    if (!ImGui::Begin(m_title.c_str(), &m_open)) { ImGui::End(); return; }

    // fg/bg swatch preview
    {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        f32 sw = 36;

        // checkerboard under bg
        for (int cy = 0; cy < 3; cy++) {
            for (int cx = 0; cx < 3; cx++) {
                bool light = ((cx + cy) & 1) == 0;
                draw->AddRectFilled(
                    ImVec2(pos.x + 6 + cx * (sw / 3), pos.y + 6 + cy * (sw / 3)),
                    ImVec2(pos.x + 6 + (cx + 1) * (sw / 3), pos.y + 6 + (cy + 1) * (sw / 3)),
                    light ? IM_COL32(55, 55, 55, 255) : IM_COL32(40, 40, 40, 255));
            }
        }

        // bg swatch
        draw->AddRectFilled(ImVec2(pos.x + 6, pos.y + 6),
            ImVec2(pos.x + sw + 6, pos.y + sw + 6),
            IM_COL32((int)(m_bg.r * 255), (int)(m_bg.g * 255), (int)(m_bg.b * 255), 255));
        draw->AddRect(ImVec2(pos.x + 6, pos.y + 6),
            ImVec2(pos.x + sw + 6, pos.y + sw + 6), IM_COL32(100, 100, 100, 255));

        // fg swatch
        draw->AddRectFilled(ImVec2(pos.x, pos.y),
            ImVec2(pos.x + sw, pos.y + sw),
            IM_COL32((int)(m_fg.r * 255), (int)(m_fg.g * 255), (int)(m_fg.b * 255), 255));
        draw->AddRect(ImVec2(pos.x, pos.y),
            ImVec2(pos.x + sw, pos.y + sw), IM_COL32(200, 200, 200, 255));

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + sw + 12);

        ImGui::SameLine(sw + 20);
        if (ImGui::Button("Swap", ImVec2(50, 24))) swapColors();
        ImGui::SameLine();
        if (ImGui::Button("Default", ImVec2(60, 24))) {
            m_fg = {0, 0, 0, 1}; m_bg = {1, 1, 1, 1};
            s_fg = m_fg; s_bg = m_bg;
        }
        ImGui::SameLine();
        ImGui::Text("FG/BG");
    }

    ImGui::Separator();

    // RGB/HSV mode toggle
    int colorMode = m_modeRGB ? 0 : 1;
    ImGui::RadioButton("RGB", &colorMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("HSV", &colorMode, 1);
    m_modeRGB = (colorMode == 0);
    ImGui::SameLine();

    // hex input
    ImGui::SetNextItemWidth(80);
    if (ImGui::InputText("##hex", m_hexBuf, sizeof(m_hexBuf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase)) {
        unsigned int hexVal;
        if (std::sscanf(m_hexBuf, "%6x", &hexVal) == 1) {
            m_fg = {
                ((hexVal >> 16) & 0xFF) / 255.0f,
                ((hexVal >> 8) & 0xFF) / 255.0f,
                (hexVal & 0xFF) / 255.0f,
                m_fg.a
            };
            s_fg = m_fg;
        }
    }

    // color pickers
    float fg[4] = {m_fg.r, m_fg.g, m_fg.b, m_fg.a};
    float bg[4] = {m_bg.r, m_bg.g, m_bg.b, m_bg.a};

    u32 flags = ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview |
                ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoSmallPreview |
                ImGuiColorEditFlags_PickerHueWheel;

    if (!m_modeRGB) {
        flags &= ~ImGuiColorEditFlags_DisplayRGB;
        flags |= ImGuiColorEditFlags_DisplayHSV;
    }

    ImGui::ColorPicker4("FG", fg, flags);
    m_fg = {fg[0], fg[1], fg[2], fg[3]};
    s_fg = m_fg;

    ImGui::ColorPicker4("BG", bg, flags);
    m_bg = {bg[0], bg[1], bg[2], bg[3]};
    s_bg = m_bg;

    ImGui::SeparatorText("Swatches");

    // swatch grid
    const int swatchSize = 18;
    const int cols = 6;
    int swatchCount = (int)m_swatches.size();

    for (int i = 0; i < swatchCount; i++) {
        if (i > 0 && i % cols != 0) ImGui::SameLine();
        if (i % cols == 0 && i > 0) ImGui::Spacing();

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* draw = ImGui::GetWindowDrawList();
        uint32_t c = m_swatches[i];
        u8 sr = (c >> 24) & 0xFF, sg = (c >> 16) & 0xFF, sb = (c >> 8) & 0xFF, sa = c & 0xFF;
        draw->AddRectFilled(p, ImVec2(p.x + swatchSize, p.y + swatchSize), IM_COL32(sr, sg, sb, sa));
        draw->AddRect(p, ImVec2(p.x + swatchSize, p.y + swatchSize), IM_COL32(80, 80, 80, 255));

        ImGui::InvisibleButton("##sw", ImVec2((f32)swatchSize, (f32)swatchSize));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            m_fg = {(f32)sr / 255, (f32)sg / 255, (f32)sb / 255, (f32)sa / 255};
            s_fg = m_fg;
            std::snprintf(m_hexBuf, sizeof(m_hexBuf), "%02X%02X%02X", sr, sg, sb);
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            m_bg = {(f32)sr / 255, (f32)sg / 255, (f32)sb / 255, (f32)sa / 255};
            s_bg = m_bg;
        }
    }

    // add current FG to swatches
    ImGui::Spacing();
    if (ImGui::Button("+ Add to Swatches", ImVec2(-1, 24))) {
        uint32_t nc = ((uint32_t)(m_fg.r * 255) << 24) |
                      ((uint32_t)(m_fg.g * 255) << 16) |
                      ((uint32_t)(m_fg.b * 255) << 8) |
                      (uint32_t)(m_fg.a * 255);
        m_swatches.push_back(nc);
    }

    ImGui::End();
}

void ColorPanel::swapColors() {
    std::swap(m_fg, m_bg);
}

// ─── Navigator ─────────────────────────────────────────────────────────────

void NavigatorPanel::render() {
    if (!m_open) return;
    if (!ImGui::Begin(m_title.c_str(), &m_open)) { ImGui::End(); return; }

    auto* doc = Application::get().activeDocument();
    if (!doc || !doc->activeComposition()) {
        ImGui::TextUnformatted("No active composition");
        ImGui::End();
        return;
    }

    auto* comp = doc->activeComposition();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    f32 navW = ImGui::GetContentRegionAvail().x;
    f32 navH = navW * ((f32)comp->height() / (f32)comp->width());
    ImVec2 navPos = ImGui::GetCursorScreenPos();

    // bg
    draw->AddRectFilled(navPos, ImVec2(navPos.x + navW, navPos.y + navH),
        IM_COL32(20, 20, 25, 255));

    // thumbnail of layers (reduced)
    f32 scale = navW / (f32)comp->width();
    for (u32 i = 0; i < comp->layerCount(); i++) {
        auto* layer = comp->layer(i);
        if (!layer->visible()) continue;
        auto& t = layer->transform();
        f32 lx = navPos.x + t.position.x * scale;
        f32 ly = navPos.y + t.position.y * scale;
        f32 lw = comp->width() * scale * t.scale.x;
        f32 lh = comp->height() * scale * t.scale.y;
        draw->AddRectFilled(ImVec2(lx, ly), ImVec2(lx + lw, ly + lh),
            IM_COL32(80 + i * 30, 60, 120, 120));
        draw->AddRect(ImVec2(lx, ly), ImVec2(lx + lw, ly + lh),
            IM_COL32(150, 200, 255, 100));
    }

    // viewport rect
    auto* canvas = Application::get().dockingSpace()
        ? Application::get().dockingSpace()->findPanel<CanvasPanel>() : nullptr;
    if (canvas) {
        f32 vx = navPos.x + (-canvas->canvasPos().x) * scale;
        f32 vy = navPos.y + (-canvas->canvasPos().y) * scale;
        f32 vw = canvas->viewport().w / canvas->zoom() * scale;
        f32 vh = canvas->viewport().h / canvas->zoom() * scale;
        draw->AddRect(ImVec2(vx, vy), ImVec2(vx + vw, vy + vh),
            IM_COL32(255, 255, 255, 160), 0, 0, 1.5f);
    }

    // click to pan
    ImGui::InvisibleButton("##nav_canvas", ImVec2(navW, navH));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && canvas) {
        f32 relX = (ImGui::GetIO().MousePos.x - navPos.x) / scale;
        f32 relY = (ImGui::GetIO().MousePos.y - navPos.y) / scale;
        f32 vw2 = canvas->viewport().w / canvas->zoom() / 2;
        f32 vh2 = canvas->viewport().h / canvas->zoom() / 2;
        canvas->setZoom(canvas->zoom());
        Vec2 cp = canvas->canvasPos();
        cp.x = -(relX - comp->width() / 2.0f) - (canvas->viewport().w / canvas->zoom() / 2 - comp->width() / 2);
        cp.y = -(relY - comp->height() / 2.0f) - (canvas->viewport().h / canvas->zoom() / 2 - comp->height() / 2);
        // we can't set canvasPos directly - it's private
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + navH);
    ImGui::Text("%d x %d", comp->width(), comp->height());

    ImGui::End();
}

// ─── Brush Settings ────────────────────────────────────────────────────────

void BrushSettingsPanel::render() {
    if (!m_open) return;
    if (!ImGui::Begin(m_title.c_str(), &m_open)) { ImGui::End(); return; }

    ImGui::SeparatorText("Brush Tip");

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("Size", &m_size, 1, 500, "%.0f px");

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("Hardness", &m_hardness, 0, 1, "%.0f%%");

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("Opacity", &m_opacity, 0, 1, "%.0f%%");

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("Flow", &m_flow, 0, 1, "%.0f%%");

    ImGui::SetNextItemWidth(-1);
    ImGui::SliderFloat("Spacing", &m_spacing, 0.01f, 1.0f, "%.0f%%");

    ImGui::SeparatorText("Blending");

    const char* blendModes[] = {"Normal", "Multiply", "Screen", "Overlay"};
    ImGui::Combo("Mode", &m_blendMode, blendModes, IM_ARRAYSIZE(blendModes));

    ImGui::SeparatorText("Dynamics");

    ImGui::SliderFloat("Size Jitter", &m_sizeJitter, 0, 1, "%.0f%%");
    ImGui::SliderFloat("Opacity Jitter", &m_opacityJitter, 0, 1, "%.0f%%");
    ImGui::Checkbox("Pressure Sensitivity", &m_usePressure);

    // brush preview
    ImGui::SeparatorText("Preview");
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 previewPos = ImGui::GetCursorScreenPos();
    f32 previewSize = 100;
    draw->AddRectFilled(previewPos, ImVec2(previewPos.x + previewSize, previewPos.y + previewSize),
        IM_COL32(30, 30, 35, 255));

    f32 cx = previewPos.x + previewSize / 2;
    f32 cy = previewPos.y + previewSize / 2;
    f32 r = (m_size / 500.0f) * (previewSize / 2 - 4);
    if (r > 2) {
        // hardness gradient (outer ring = hardness, inner = soft)
        i32 steps = 20;
        for (i32 si = steps; si >= 0; si--) {
            f32 t = (f32)si / (f32)steps;
            f32 radius = r * t;
            u8 alpha = (u8)(255 * (1.0f - std::pow(t, 1.0f + (1.0f - m_hardness) * 4.0f)));
            draw->AddCircleFilled(ImVec2(cx, cy), radius, IM_COL32(200, 200, 200, alpha));
        }
    } else {
        draw->AddCircleFilled(ImVec2(cx, cy), 2, IM_COL32(200, 200, 200, 255));
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + previewSize + 4);
    ImGui::ProgressBar(m_size / 500.0f, ImVec2(-1, 8));
    ImGui::Text("Size: %.0f | Hardness: %.0f%%", m_size, m_hardness * 100);

    ImGui::End();
}
