#pragma once
#include "../core/Types.h"
#include "../core/Math.h"
#include "VertexArray.h"
#include <vector>

class Mesh {
public:
    Mesh() = default;

    void createFromTriangles(const std::vector<Vec2>& verts, const std::vector<u32>& indices);
    void createFromRect(const Rect& rect);

    const VertexArray& va() const { return m_va; }
    u32 indexCount() const { return m_va.indexCount(); }

private:
    VertexArray m_va;
};

struct MeshData {
    std::vector<f32> vertices;
    std::vector<u32> indices;

    void quad(float x, float y, float w, float h, const ColorRGBA& color = {1,1,1,1});
    void clear();
};
