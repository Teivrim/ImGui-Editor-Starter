#include "Mesh.h"
#include "GL.h"

void Mesh::createFromTriangles(const std::vector<Vec2>& verts, const std::vector<u32>& indices) {
    std::vector<f32> data;
    data.reserve(verts.size() * 7);
    for (const auto& v : verts) {
        data.push_back(v.x); data.push_back(v.y); data.push_back(0);
        data.push_back(1); data.push_back(1); data.push_back(1); data.push_back(1);
    }
    m_va.setVertexBuffer(data.data(), (u32)(data.size() * sizeof(f32)), {
        {0, 3, GL_FLOAT, 7 * sizeof(f32), 0},
        {1, 4, GL_FLOAT, 7 * sizeof(f32), 3 * sizeof(f32)}
    });
    m_va.setIndexBuffer(indices.data(), (u32)indices.size());
}

void Mesh::createFromRect(const Rect& rect) {
    std::vector<f32> verts = {
        rect.x, rect.y, 0, 1,1,1,1,
        rect.x + rect.w, rect.y, 0, 1,1,1,1,
        rect.x + rect.w, rect.y + rect.h, 0, 1,1,1,1,
        rect.x, rect.y + rect.h, 0, 1,1,1,1,
    };
    std::vector<u32> idx = {0,1,2,0,2,3};
    m_va.setVertexBuffer(verts.data(), (u32)(verts.size() * sizeof(f32)), {
        {0, 3, GL_FLOAT, 7 * sizeof(f32), 0},
        {1, 4, GL_FLOAT, 7 * sizeof(f32), 3 * sizeof(f32)}
    });
    m_va.setIndexBuffer(idx.data(), (u32)idx.size());
}

void MeshData::quad(float x, float y, float w, float h, const ColorRGBA& color) {
    u32 base = (u32)(vertices.size() / 7);
    vertices.insert(vertices.end(), {
        x, y, 0, color.r, color.g, color.b, color.a,
        x+w, y, 0, color.r, color.g, color.b, color.a,
        x+w, y+h, 0, color.r, color.g, color.b, color.a,
        x, y+h, 0, color.r, color.g, color.b, color.a,
    });
    indices.insert(indices.end(), {base, base+1, base+2, base, base+2, base+3});
}

void MeshData::clear() { vertices.clear(); indices.clear(); }
