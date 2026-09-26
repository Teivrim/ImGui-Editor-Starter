#include "core/Application.h"
#include "core/Logger.h"
#include "core/Types.h"
#include "core/Math.h"
#include "document/Document.h"
#include "document/Composition.h"
#include "document/Layer.h"
#include "ui/DockingSpace.h"
#include "ui/Panel.h"
#include "ui/Style.h"
#include "undo/UndoStack.h"
#include "render/Renderer.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <memory>
#include <cstdio>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <windows.h>

#include <stb_image.h>
#include <stb_image_write.h>

class EditorApp : public Application {
protected:
    void onInit() override {
        SetupEditorStyle();

        auto doc = std::make_unique<Document>("My Project");
        auto comp = std::make_unique<Composition>("Main Comp", 1920, 1080);
        comp->setFrameRate(30.0);
        comp->setDuration(30.0);

        auto bgLayer = std::make_unique<PixelLayer>();
        bgLayer->setName("Background");
        bgLayer->create(1920, 1080);

        auto* pixels = bgLayer->pixels();
        for (u32 y = 0; y < 1080; y++) {
            for (u32 x = 0; x < 1920; x++) {
                u32 idx = (y * 1920 + x) * 4;
                f32 cx = (f32)x / 1920.0f;
                f32 cy = (f32)y / 1080.0f;
                pixels[idx]     = (u8)(cx * 255);
                pixels[idx + 1] = (u8)(cy * 255);
                pixels[idx + 2] = (u8)((1 - cx) * 255);
                pixels[idx + 3] = 255;
            }
        }
        bgLayer->uploadTexture();
        comp->addLayer(std::move(bgLayer));

        auto shapeLayer = std::make_unique<PixelLayer>();
        shapeLayer->setName("Shape");
        shapeLayer->create(400, 400);

        pixels = shapeLayer->pixels();
        for (u32 y = 0; y < 400; y++) {
            for (u32 x = 0; x < 400; x++) {
                u32 idx = (y * 400 + x) * 4;
                f32 dx = (f32)x / 400.0f - 0.5f;
                f32 dy = (f32)y / 400.0f - 0.5f;
                if (dx*dx + dy*dy < 0.2f) {
                    pixels[idx] = 255; pixels[idx+1] = 100;
                    pixels[idx+2] = 50; pixels[idx+3] = 200;
                }
            }
        }
        shapeLayer->uploadTexture();
        shapeLayer->transform().position = {800, 300};
        comp->addLayer(std::move(shapeLayer));

        doc->addComposition(std::move(comp));
        setActiveDocument(std::move(doc));

        m_docking = std::make_unique<DockingSpace>();
        m_docking->addPanel(std::make_unique<CanvasPanel>());
        m_docking->addPanel(std::make_unique<LayersPanel>());
        m_docking->addPanel(std::make_unique<ToolsPanel>());
        m_docking->addPanel(std::make_unique<BrushSettingsPanel>());
        m_docking->addPanel(std::make_unique<TimelinePanel>());
        m_docking->addPanel(std::make_unique<PropertiesPanel>());
        m_docking->addPanel(std::make_unique<HistoryPanel>());
        m_docking->addPanel(std::make_unique<ColorPanel>());
        m_docking->addPanel(std::make_unique<NavigatorPanel>());
        setDockingSpace(m_docking.get());

        loadRecentFiles();
    }

    void onUpdate(f64 dt) override {
        if (m_docking) {
            auto* tl = m_docking->findPanel<TimelinePanel>();
            if (tl && tl->isPlaying()) {
                auto* doc = activeDocument();
                if (doc && doc->activeComposition()) {
                    f64 newTime = tl->currentTime() + dt;
                    if (newTime > doc->activeComposition()->duration())
                        newTime = 0;
                    tl->setCurrentTime(newTime);
                }
            }
        }
    }

    void onRender() override {
        auto* doc = activeDocument();
        if (doc && doc->activeComposition()) {
            renderer().beginFrame(0.1f, 0.1f, 0.12f, 1.0f);
            doc->activeComposition()->render(renderer());
        }
    }

    void handleShortcuts() {
        auto& io = ImGui::GetIO();
        if (io.WantTextInput) return;

        bool ctrl = io.KeyCtrl;
        bool shift = io.KeyShift;

        // File shortcuts
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_N, false)) { requestAction(PendingAction::NewProject); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_O, false)) { requestAction(PendingAction::OpenProject); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_I, false)) { importImage(); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            if (m_projectPath.empty()) saveProjectAs();
            else saveProject(m_projectPath);
        }
        if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_S, false)) { saveProjectAs(); }

        // Edit shortcuts
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Z, false)) { undoStack().undo(); }
        if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_Z, false)) { undoStack().redo(); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_X, false)) { Logger::instance().info("Cut"); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_C, false)) { Logger::instance().info("Copy"); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_V, false)) { Logger::instance().info("Paste"); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_A, false)) { Logger::instance().info("Select All"); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_D, false)) { Logger::instance().info("Deselect"); }
        if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_I, false)) { Logger::instance().info("Inverse Selection"); }

        // View shortcuts
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Equal, false)) {
            if (auto* canvas = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr) {
                auto z = canvas->zoom(); canvas->setZoom(z * 1.2f);
            }
        }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Minus, false)) {
            if (auto* canvas = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr) {
                auto z = canvas->zoom(); canvas->setZoom(z / 1.2f);
            }
        }

        // Layer shortcuts
        if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_N, false)) { newLayer(); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_J, false)) { duplicateLayer(); }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_E, false)) { Logger::instance().info("Merge Down"); }

        // Tool shortcuts (V, M, B, E, P, U, T, C, I, Z, H)
        if (!ctrl && !shift) {
            auto* tools = m_docking ? m_docking->findPanel<ToolsPanel>() : nullptr;
            if (tools) {
                if (ImGui::IsKeyPressed(ImGuiKey_V, false)) tools->setActiveTool(ToolType::Select);
                if (ImGui::IsKeyPressed(ImGuiKey_M, false)) tools->setActiveTool(ToolType::Move);
                if (ImGui::IsKeyPressed(ImGuiKey_B, false)) tools->setActiveTool(ToolType::Brush);
                if (ImGui::IsKeyPressed(ImGuiKey_E, false)) tools->setActiveTool(ToolType::Eraser);
                if (ImGui::IsKeyPressed(ImGuiKey_P, false)) tools->setActiveTool(ToolType::Pen);
                if (ImGui::IsKeyPressed(ImGuiKey_U, false)) tools->setActiveTool(ToolType::Shape);
                if (ImGui::IsKeyPressed(ImGuiKey_T, false)) tools->setActiveTool(ToolType::Text);
                if (ImGui::IsKeyPressed(ImGuiKey_C, false)) tools->setActiveTool(ToolType::Crop);
                if (ImGui::IsKeyPressed(ImGuiKey_I, false)) tools->setActiveTool(ToolType::Eyedropper);
                if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) tools->setActiveTool(ToolType::Zoom);
                if (ImGui::IsKeyPressed(ImGuiKey_H, false)) tools->setActiveTool(ToolType::Hand);
            }
        }

        // View toggles
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_R, false)) {
            if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                cv->setShowRulers(!cv->showRulers());
        }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Apostrophe, false)) {
            if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                cv->setShowGrid(!cv->showGrid());
        }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Semicolon, false)) {
            if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                cv->setSnapEnabled(!cv->snapEnabled());
        }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_0, false)) {
            if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                cv->fitToScreen();
        }
        if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_1, false)) {
            if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                cv->setZoom(1.0f);
        }

        // Delete selected layer
        if (!ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_Delete, false)) {
            auto* d = activeDocument();
            if (d && d->activeComposition()) {
                auto* c = d->activeComposition();
                if (c->layerCount() > 0) {
                    c->removeLayer(c->selectedLayerIndex());
                    d->setModified(true);
                }
            }
        }

        // Performance window
        if (ImGui::IsKeyPressed(ImGuiKey_F11, false)) m_showPerf = !m_showPerf;
    }

    void newProject() {
        undoStack().clear();
        auto doc = std::make_unique<Document>("Untitled");
        auto comp = std::make_unique<Composition>("Main Comp", 1920, 1080);
        comp->setFrameRate(30.0);
        comp->setDuration(30.0);

        auto bg = std::make_unique<PixelLayer>();
        bg->setName("Background");
        bg->create(1920, 1080);
        comp->addLayer(std::move(bg));

        doc->addComposition(std::move(comp));
        setActiveDocument(std::move(doc));
        m_projectPath.clear();
        window().setTitle("Editor Core - Untitled");
        Logger::instance().info("New project created");
    }

    void openProject() {
        char path[MAX_PATH] = {0};
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFilter = "EditorCore Project\0*.ecp\0All Files\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

        if (GetOpenFileNameA(&ofn)) {
            loadFile(path);
        }
    }

    void saveProjectAs() {
        char path[MAX_PATH] = {0};
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFilter = "EditorCore Project\0*.ecp\0All Files\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = "ecp";
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

        if (GetSaveFileNameA(&ofn)) {
            saveProject(path);
        }
    }

    void saveProject(const std::string& path) {
        auto* doc = activeDocument();
        if (!doc) return;

        std::ofstream out(path, std::ios::binary);
        if (!out) { Logger::instance().error("Failed to save: " + path); return; }

        u32 magic = 0x45435046; // "ECPF"
        out.write((const char*)&magic, sizeof(magic));
        u32 ver = 1;
        out.write((const char*)&ver, sizeof(ver));

        u32 nameLen = (u32)doc->name().size();
        out.write((const char*)&nameLen, sizeof(nameLen));
        out.write(doc->name().data(), nameLen);

        u32 compCount = doc->compositionCount();
        out.write((const char*)&compCount, sizeof(compCount));

        for (u32 ci = 0; ci < compCount; ci++) {
            auto* comp = doc->composition(ci);
            u32 cnLen = (u32)comp->name().size();
            out.write((const char*)&cnLen, sizeof(cnLen));
            out.write(comp->name().data(), cnLen);
            u32 cw = comp->width(), ch = comp->height();
            out.write((const char*)&cw, sizeof(cw));
            out.write((const char*)&ch, sizeof(ch));
            f64 fr = comp->frameRate(), dur = comp->duration();
            out.write((const char*)&fr, sizeof(fr));
            out.write((const char*)&dur, sizeof(dur));

            u32 lc = comp->layerCount();
            out.write((const char*)&lc, sizeof(lc));
            for (u32 li = 0; li < lc; li++) {
                auto* layer = comp->layer(li);
                u8 lt = (u8)layer->type();
                out.write((const char*)&lt, sizeof(lt));
                u32 lnLen = (u32)layer->name().size();
                out.write((const char*)&lnLen, sizeof(lnLen));
                out.write(layer->name().data(), lnLen);
                bool vis = layer->visible();
                out.write((const char*)&vis, sizeof(vis));
                f32 op = layer->opacity();
                out.write((const char*)&op, sizeof(op));
                u8 bm = (u8)layer->blendMode();
                out.write((const char*)&bm, sizeof(bm));
                auto& t = layer->transform();
                out.write((const char*)&t.position, sizeof(t.position));
                out.write((const char*)&t.scale, sizeof(t.scale));
                out.write((const char*)&t.rotation, sizeof(t.rotation));
                out.write((const char*)&t.anchor, sizeof(t.anchor));

                // pixel layer data
                if (layer->type() == LayerType::Pixel) {
                    auto* pl = static_cast<PixelLayer*>(layer);
                    u32 pw = pl->width(), ph = pl->height();
                    out.write((const char*)&pw, sizeof(pw));
                    out.write((const char*)&ph, sizeof(ph));
                    if (pl->pixels()) {
                        out.write((const char*)pl->pixels(), pw * ph * 4);
                    }
                }
            }
        }

        out.close();
        m_projectPath = path;
        doc->setModified(false);
        window().setTitle("Editor Core - " + std::filesystem::path(path).filename().string());
        Logger::instance().info("Project saved: " + path);
    }

    void loadFile(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) { Logger::instance().error("Failed to load: " + path); return; }

        u32 magic;
        in.read((char*)&magic, sizeof(magic));
        if (magic != 0x45435046) { Logger::instance().error("Invalid project file"); return; }

        u32 ver;
        in.read((char*)&ver, sizeof(ver));

        u32 nameLen;
        in.read((char*)&nameLen, sizeof(nameLen));
        std::string docName(nameLen, '\0');
        in.read(docName.data(), nameLen);

        auto doc = std::make_unique<Document>(docName);

        u32 compCount;
        in.read((char*)&compCount, sizeof(compCount));
        for (u32 ci = 0; ci < compCount; ci++) {
            u32 cnLen;
            in.read((char*)&cnLen, sizeof(cnLen));
            std::string compName(cnLen, '\0');
            in.read(compName.data(), cnLen);

            u32 cw, ch;
            in.read((char*)&cw, sizeof(cw));
            in.read((char*)&ch, sizeof(ch));
            f64 fr, dur;
            in.read((char*)&fr, sizeof(fr));
            in.read((char*)&dur, sizeof(dur));

            auto comp = std::make_unique<Composition>(compName, cw, ch);
            comp->setFrameRate(fr);
            comp->setDuration(dur);

            u32 lc;
            in.read((char*)&lc, sizeof(lc));
            for (u32 li = 0; li < lc; li++) {
                u8 lt;
                in.read((char*)&lt, sizeof(lt));
                u32 lnLen;
                in.read((char*)&lnLen, sizeof(lnLen));
                std::string lName(lnLen, '\0');
                in.read(lName.data(), lnLen);
                bool vis;
                in.read((char*)&vis, sizeof(vis));
                f32 op;
                in.read((char*)&op, sizeof(op));
                u8 bm;
                in.read((char*)&bm, sizeof(bm));

                Transform t;
                in.read((char*)&t.position, sizeof(t.position));
                in.read((char*)&t.scale, sizeof(t.scale));
                in.read((char*)&t.rotation, sizeof(t.rotation));
                in.read((char*)&t.anchor, sizeof(t.anchor));

                std::unique_ptr<Layer> layer;
                if ((LayerType)lt == LayerType::Pixel) {
                    auto pl = std::make_unique<PixelLayer>();
                    u32 pw, ph;
                    in.read((char*)&pw, sizeof(pw));
                    in.read((char*)&ph, sizeof(ph));
                    pl->create(pw, ph);
                    if (pw > 0 && ph > 0) {
                        in.read((char*)pl->pixels(), pw * ph * 4);
                        pl->uploadTexture();
                    }
                    layer = std::move(pl);
                } else {
                    layer = std::make_unique<PixelLayer>();
                }

                layer->setName(lName);
                layer->setVisible(vis);
                layer->setOpacity(op);
                layer->setBlendMode((BlendMode)bm);
                layer->transform() = t;

                comp->addLayer(std::move(layer));
            }

            doc->addComposition(std::move(comp));
        }

        in.close();
        setActiveDocument(std::move(doc));
        m_projectPath = path;
        window().setTitle("Editor Core - " + std::filesystem::path(path).filename().string());
        Logger::instance().info("Project loaded: " + path);
    }

    void newLayer() {
        auto* doc = activeDocument();
        if (!doc || !doc->activeComposition()) return;
        auto* comp = doc->activeComposition();
        auto layer = std::make_unique<PixelLayer>();
        layer->setName("Layer " + std::to_string(comp->layerCount() + 1));
        layer->create(comp->width(), comp->height());
        comp->addLayer(std::move(layer));
        doc->setModified(true);
        Logger::instance().info("New layer added via shortcut");
    }

    void duplicateLayer() {
        auto* doc = activeDocument();
        if (!doc || !doc->activeComposition()) return;
        auto* comp = doc->activeComposition();
        if (comp->layerCount() == 0) return;
        auto* src = comp->layer(comp->layerCount() - 1);
        auto dup = std::make_unique<PixelLayer>();
        dup->setName(src->name() + " copy");
        dup->setVisible(src->visible());
        dup->setOpacity(src->opacity());
        dup->setBlendMode(src->blendMode());
        dup->transform() = src->transform();
        if (auto* pl = dynamic_cast<PixelLayer*>(src)) {
            dup->create(pl->width(), pl->height(), pl->pixels());
            dup->uploadTexture();
        }
        comp->addLayer(std::move(dup));
        doc->setModified(true);
        Logger::instance().info("Layer duplicated");
    }

    void importImage() {
        char path[MAX_PATH] = {0};
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFilter = "Images\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0All Files\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

        if (!GetOpenFileNameA(&ofn)) return;

        int w, h, chan;
        u8* img = stbi_load(path, &w, &h, &chan, 4);
        if (!img) {
            Logger::instance().error("Failed to load image: " + std::string(path));
            return;
        }

        auto* doc = activeDocument();
        if (!doc || !doc->activeComposition()) {
            stbi_image_free(img);
            return;
        }
        auto* activeComp = doc->activeComposition();

        auto layer = std::make_unique<PixelLayer>();
        layer->setName(std::filesystem::path(path).stem().string());
        layer->create((u32)w, (u32)h, img);
        layer->uploadTexture();
        activeComp->addLayer(std::move(layer));

        stbi_image_free(img);
        Logger::instance().info("Imported: " + std::string(path));
        addRecentFile(path);
    }

    void exportPNG() {
        char path[MAX_PATH] = {0};
        OPENFILENAMEA ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = GetActiveWindow();
        ofn.lpstrFilter = "PNG Image\0*.png\0All Files\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = "png";
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

        if (!GetSaveFileNameA(&ofn)) return;

        auto* doc = activeDocument();
        if (!doc || !doc->activeComposition()) return;
        auto* comp = doc->activeComposition();
        auto* layer = comp->selectedLayer();
        if (!layer) return;
        auto* pl = dynamic_cast<PixelLayer*>(layer);
        if (!pl || !pl->pixels()) return;

        int success = stbi_write_png(path, (int)pl->width(), (int)pl->height(), 4, pl->pixels(), (int)pl->width() * 4);
        if (success)
            Logger::instance().info("Exported PNG: " + std::string(path));
        else
            Logger::instance().error("Failed to export PNG");
    }

    // ── Recent Files ──
    void addRecentFile(const std::string& path) {
        auto it = std::find(m_recentFiles.begin(), m_recentFiles.end(), path);
        if (it != m_recentFiles.end()) m_recentFiles.erase(it);
        m_recentFiles.insert(m_recentFiles.begin(), path);
        if (m_recentFiles.size() > 10) m_recentFiles.resize(10);
        saveRecentFiles();
    }

    void saveRecentFiles() {
        std::string iniPath = std::filesystem::path(m_projectPath).parent_path().string();
        if (iniPath.empty()) iniPath = ".";
        std::ofstream out(iniPath + "\\recent.ecp");
        for (auto& f : m_recentFiles) out << f << "\n";
    }

    void loadRecentFiles() {
        m_recentFiles.clear();
        std::ifstream in("recent.ecp");
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) m_recentFiles.push_back(line);
        }
    }

    void onUI() override {
        const f32 menuH = ImGui::GetFrameHeight();
        const f32 statusH = 24;

        handleShortcuts();

        // ── Menu Bar ──
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Project", "Ctrl+N")) { requestAction(PendingAction::NewProject); }
                if (ImGui::MenuItem("Open...", "Ctrl+O")) { requestAction(PendingAction::OpenProject); }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    if (m_projectPath.empty()) saveProjectAs();
                    else saveProject(m_projectPath);
                }
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) { saveProjectAs(); }
                ImGui::Separator();
                if (ImGui::MenuItem("Open Image...", "Ctrl+I")) { importImage(); }
                if (ImGui::BeginMenu("Export")) {
                    if (ImGui::MenuItem("PNG...")) { exportPNG(); }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (!m_recentFiles.empty()) {
                    if (ImGui::BeginMenu("Recent Files")) {
                        for (auto& rf : m_recentFiles) {
                            if (ImGui::MenuItem(rf.c_str())) {
                                loadFile(rf);
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::Separator();
                }
                if (ImGui::MenuItem("Exit", "Alt+F4")) { requestAction(PendingAction::Exit); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) undoStack().undo();
                if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z")) undoStack().redo();
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
                if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
                if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                    if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                        cv->setZoom(cv->zoom() * 1.2f);
                }
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                        cv->setZoom(cv->zoom() / 1.2f);
                }
                if (ImGui::MenuItem("Fit to Screen", "Ctrl+0")) {
                    if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                        cv->fitToScreen();
                }
                if (ImGui::MenuItem("Actual Size", "Ctrl+1")) {
                    if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr)
                        cv->setZoom(1.0f);
                }
                ImGui::Separator();
                if (auto* cv = m_docking ? m_docking->findPanel<CanvasPanel>() : nullptr) {
                    bool rulers = cv->showRulers();
                    if (ImGui::MenuItem("Rulers", nullptr, &rulers)) cv->setShowRulers(rulers);
                    bool grid = cv->showGrid();
                    if (ImGui::MenuItem("Grid", nullptr, &grid)) cv->setShowGrid(grid);
                    bool snap = cv->snapEnabled();
                    if (ImGui::MenuItem("Snap", nullptr, &snap)) cv->setSnapEnabled(snap);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Layout")) {
                    ImGui::DockBuilderRemoveNode(ImGui::GetID("MainDockspace"));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Select")) {
                if (ImGui::MenuItem("All", "Ctrl+A")) {}
                if (ImGui::MenuItem("Deselect", "Ctrl+D")) {}
                if (ImGui::MenuItem("Inverse", "Ctrl+Shift+I")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Similar")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Layer")) {
                if (ImGui::MenuItem("New Layer", "Ctrl+Shift+N")) { newLayer(); }
                if (ImGui::MenuItem("Duplicate Layer", "Ctrl+J")) { duplicateLayer(); }
                if (ImGui::MenuItem("Delete Layer", "Del")) {
                    auto* d = activeDocument();
                    if (d && d->activeComposition()) {
                        auto* c = d->activeComposition();
                        if (c->layerCount() > 0) {
                            c->removeLayer(c->selectedLayerIndex());
                            d->setModified(true);
                        }
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Rename Layer")) {}
                if (ImGui::MenuItem("Layer Properties...")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Merge Down", "Ctrl+E")) {}
                if (ImGui::MenuItem("Merge Visible")) {}
                if (ImGui::MenuItem("Flatten Image")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("New Group")) {
                    auto* d = activeDocument();
                    if (d && d->activeComposition()) {
                        auto* c = d->activeComposition();
                        auto g = std::make_unique<GroupLayer>();
                        g->setName("Group " + std::to_string(c->layerCount() + 1));
                        c->addLayer(std::move(g));
                    }
                }
                if (ImGui::BeginMenu("Lock")) {
                    auto* d = activeDocument();
                    auto* sel = d && d->activeComposition() ? d->activeComposition()->selectedLayer() : nullptr;
                    bool lk = sel ? sel->locked() : false;
                    if (ImGui::MenuItem("Lock Selected", nullptr, &lk)) { if (sel) sel->setLocked(lk); }
                    if (ImGui::MenuItem("Unlock All")) {
                        if (d && d->activeComposition())
                            for (u32 li = 0; li < d->activeComposition()->layerCount(); li++)
                                d->activeComposition()->layer(li)->setLocked(false);
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Filter")) {
                if (ImGui::MenuItem("Blur")) {}
                if (ImGui::MenuItem("Brightness/Contrast")) {}
                if (ImGui::MenuItem("Color Balance")) {}
                if (ImGui::MenuItem("Hue/Saturation")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) {
                auto togglePanel = [&](const char* name, Panel* p) {
                    if (!p) return;
                    bool open = p->isOpen();
                    if (ImGui::MenuItem(name, nullptr, &open)) p->setOpen(open);
                };
                if (m_docking) {
                    togglePanel("Canvas", m_docking->findPanel<CanvasPanel>());
                    togglePanel("Layers", m_docking->findPanel<LayersPanel>());
                    togglePanel("Tools", m_docking->findPanel<ToolsPanel>());
                    togglePanel("Brush", m_docking->findPanel<BrushSettingsPanel>());
                    togglePanel("Timeline", m_docking->findPanel<TimelinePanel>());
                    togglePanel("Properties", m_docking->findPanel<PropertiesPanel>());
                    togglePanel("History", m_docking->findPanel<HistoryPanel>());
                    togglePanel("Color", m_docking->findPanel<ColorPanel>());
                    togglePanel("Navigator", m_docking->findPanel<NavigatorPanel>());
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Layout")) {
                    ImGui::DockBuilderRemoveNode(ImGui::GetID("MainDockspace"));
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {}
                if (ImGui::MenuItem("Performance Window", "F11", m_showPerf)) {
                    m_showPerf = !m_showPerf;
                }
                ImGui::EndMenu();
            }

            // center: project name
            ImGui::SameLine(ImGui::GetWindowWidth() / 2 - 80);
            auto* doc = activeDocument();
            if (doc) {
                ImGui::Text("  %s%s", doc->name().c_str(), doc->modified() ? " *" : "");
            } else {
                ImGui::Text("  No project");
            }

            ImGui::EndMainMenuBar();
        }

        // ── Composition tabs ──
        auto* doc = activeDocument();
        if (doc && doc->compositionCount() > 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 3));
            if (ImGui::BeginViewportSideBar("##CompTabs", ImGui::GetMainViewport(),
                    ImGuiDir_Up, 28 + menuH, ImGuiWindowFlags_NoScrollbar)) {
                ImGui::SetCursorPosY(menuH + 2);
                f32 availW = ImGui::GetContentRegionAvail().x;

                if (ImGui::BeginTabBar("Compositions", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
                    for (u32 ci = 0; ci < doc->compositionCount(); ci++) {
                        auto* comp = doc->composition(ci);
                        bool isActive = (comp == doc->activeComposition());
                        ImGuiTabItemFlags flags = isActive ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;

                        bool open = true;
                        if (ImGui::BeginTabItem(comp->name().c_str(), &open, flags)) {
                            if (!isActive) doc->setActiveComposition(ci);
                            ImGui::EndTabItem();
                        }

                        if (!open) {
                            doc->removeComposition(ci);
                            break;
                        }
                    }

                    // + button for new comp
                    ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip);
                    if (ImGui::IsItemClicked()) {
                        auto newComp = std::make_unique<Composition>("Comp " + std::to_string(doc->compositionCount() + 1), 1920, 1080);
                        newComp->setFrameRate(30.0);
                        newComp->setDuration(30.0);
                        auto bg = std::make_unique<PixelLayer>();
                        bg->setName("Background");
                        bg->create(1920, 1080);
                        newComp->addLayer(std::move(bg));
                        doc->addComposition(std::move(newComp));
                        doc->setActiveComposition(doc->compositionCount() - 1);
                    }

                    ImGui::EndTabBar();
                }

                ImGui::End();
            }
            ImGui::PopStyleVar();
        }

        // ── Tool bar ──
        if (m_docking) {
            auto* tools = m_docking->findPanel<ToolsPanel>();
            if (tools) {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 3));
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.14f, 1));

                if (ImGui::BeginViewportSideBar("##Toolbar", ImGui::GetMainViewport(),
                        ImGuiDir_Up, 32 + menuH, ImGuiWindowFlags_NoScrollbar)) {
                    ImGui::SetCursorPosY(menuH + 4);

                    const char* toolShort[] = {"V","M","B","E","P","U","T","C","I","Z","H"};
                    int current = (int)tools->activeTool();
                    for (int i = 0; i < IM_ARRAYSIZE(toolShort); i++) {
                        if (i > 0) ImGui::SameLine(0, 2);
                        ImVec4 c = (current == i) ? ImVec4(0.3f,0.5f,0.8f,1) : ImVec4(0.15f,0.15f,0.17f,1);
                        ImGui::PushStyleColor(ImGuiCol_Button, c);
                        if (ImGui::Button(toolShort[i], ImVec2(28, 26)))
                            tools->setActiveTool((ToolType)i);
                        ImGui::PopStyleColor();
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("%s", toolShort[i]);
                    }

                    ImGui::SameLine(ImGui::GetWindowWidth() - 180);
                    ImGui::Text("Editor Core v%s", desc().version.c_str());

                    ImGui::End();
                }

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
            }
        }

        // ── Docking panels ──
        if (m_docking) m_docking->render();

        // ── Performance ──
        if (m_showPerf) {
            ImGui::Begin("Performance", &m_showPerf);
            ImGui::Text("FPS: %u", fps());
            ImGui::Text("Frame Time: %.3f ms", frameTime() * 1000.0);
            ImGui::Text("Delta Time: %.3f ms", deltaTime() * 1000.0);
            ImGui::Text("Task Queue: %u", taskScheduler().pendingTasks());
            ImGui::Separator();
            ImGui::Text("Window: %dx%d", window().width(), window().height());
            ImGui::Text("DPI: %.1f", window().dpiScale());
            ImGui::SeparatorText("Active Document");
            if (auto* d = activeDocument()) {
                ImGui::Text("Path: %s", m_projectPath.empty() ? "(unsaved)" : m_projectPath.c_str());
                ImGui::Text("Undo commands: %u", undoStack().commandCount());
                if (auto* comp = d->activeComposition()) {
                    ImGui::Text("Compositions: %u", d->compositionCount());
                    ImGui::Text("Layers: %u", comp->layerCount());
                }
            }
            ImGui::End();
        }

        // ── Status Bar ──
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 2));
            if (ImGui::BeginViewportSideBar("##StatusBar", ImGui::GetMainViewport(),
                    ImGuiDir_Down, statusH, ImGuiWindowFlags_NoScrollbar)) {
                ImGui::SetCursorPosY(2);
                ImGui::Text("FPS: %u", fps());

                ImGui::SameLine(120);
                if (auto* d = activeDocument()) {
                    if (auto* comp = d->activeComposition()) {
                        ImGui::Text("|  %ux%u  |  %.1f fps",
                            comp->width(), comp->height(), comp->frameRate());
                    }
                }

                ImGui::SameLine(320);
                if (auto* tools = m_docking ? m_docking->findPanel<ToolsPanel>() : nullptr) {
                    ImGui::Text("|  Tool: %s", (int)tools->activeTool() == 0 ? "Select" :
                        (int)tools->activeTool() == 1 ? "Move" :
                        (int)tools->activeTool() == 2 ? "Brush" :
                        (int)tools->activeTool() == 3 ? "Eraser" :
                        (int)tools->activeTool() == 4 ? "Pen" :
                        (int)tools->activeTool() == 5 ? "Shape" :
                        (int)tools->activeTool() == 6 ? "Text" :
                        (int)tools->activeTool() == 7 ? "Crop" :
                        (int)tools->activeTool() == 8 ? "Eyedropper" :
                        (int)tools->activeTool() == 9 ? "Zoom" : "Hand");
                }

                ImGui::SameLine(320);
                const char* toolDesc[] = {"Select/Move", "Move Layer", "Paint Brush", "Eraser", "Pen", "Shape",
                    "Text", "Crop", "Eyedropper", "Zoom", "Hand/Pan"};
                if (m_docking) {
                    auto* tools = m_docking->findPanel<ToolsPanel>();
                    if (tools) {
                        int t = (int)tools->activeTool();
                        ImGui::Text("|  %s  |", t >= 0 && t < 11 ? toolDesc[t] : "?");
                    }
                }

                ImGui::SameLine(500);
                auto cursorPos = window().mousePos();
                ImGui::Text("|  X:%.0f Y:%.0f", cursorPos.x, cursorPos.y);

                ImGui::SameLine(ImGui::GetWindowWidth() - 220);
                auto* d = activeDocument();
                u32 layerCount = (d && d->activeComposition()) ? d->activeComposition()->layerCount() : 0;
                ImGui::Text("|  L:%u  %s  v%s",
                    layerCount,
                    m_projectPath.empty() ? "unsaved" : std::filesystem::path(m_projectPath).filename().string().c_str(),
                    desc().version.c_str());

                ImGui::End();
            }
            ImGui::PopStyleVar();
        }

        // ── Save confirmation modal ──
        renderSaveDialog();
    }

private:
    enum class PendingAction { None, NewProject, OpenProject, Exit };

    std::unique_ptr<DockingSpace> m_docking;
    bool m_showPerf = false;
    std::string m_projectPath;
    std::vector<std::string> m_recentFiles;
    PendingAction m_pendingAction = PendingAction::None;

    bool hasUnsavedChanges() {
        auto* doc = activeDocument();
        return doc && doc->modified();
    }

    void requestAction(PendingAction action) {
        if (hasUnsavedChanges()) {
            m_pendingAction = action;
        } else {
            executePendingAction(action);
        }
    }

    void executePendingAction(PendingAction action) {
        switch (action) {
            case PendingAction::NewProject: newProject(); break;
            case PendingAction::OpenProject: openProject(); break;
            case PendingAction::Exit: window().close(); break;
            default: break;
        }
        m_pendingAction = PendingAction::None;
    }

    void renderSaveDialog() {
        if (m_pendingAction == PendingAction::None) return;

        ImGui::OpenPopup("Unsaved Changes");
        ImGui::SetNextWindowSize(ImVec2(360, 0));
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0, 0, 0, 0.5f));
        if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_NoResize)) {
            ImGui::TextWrapped("The current project has unsaved changes.");
            ImGui::TextWrapped("Do you want to save them before continuing?");
            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::Button("Save", ImVec2(100, 26))) {
                if (m_projectPath.empty()) saveProjectAs();
                else saveProject(m_projectPath);
                ImGui::CloseCurrentPopup();
                executePendingAction(m_pendingAction);
            }
            ImGui::SameLine();
            if (ImGui::Button("Don't Save", ImVec2(100, 26))) {
                ImGui::CloseCurrentPopup();
                executePendingAction(m_pendingAction);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 26))) {
                ImGui::CloseCurrentPopup();
                m_pendingAction = PendingAction::None;
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor();
    }
};

int main() {
    ApplicationDesc desc;
    desc.name = "Editor Core";
    desc.version = "1.0.0";
    desc.width = 1600;
    desc.height = 900;
    desc.vsync = true;
    desc.enableDocking = true;

    auto& app = EditorApp::get();
    if (!app.init(desc)) return 1;
    app.run();
    app.shutdown();
    return 0;
}
