#include "Renderer/TerrainRenderer.h"
#include "Renderer/TerrainManager.h"

#include <vector>
#include <cstdint>

static std::vector<float> g_vertices;

const std::vector<float>& TerrainRenderer::GetVertices()
{
    return g_vertices;
}

void TerrainRenderer::Build()
{
    g_vertices.clear();
    if (!TerrainManager::HasTerrain())
        return;

    const TerrainTile* tiles = TerrainManager::GetTilesConst();
    const uint32_t     tileCount = TerrainManager::GetTileCount();
    if (!tiles || tileCount == 0)
        return;

    for (uint32_t i = 0; i < tileCount; ++i)
    {
        const TerrainTile& tile = tiles[i];
        const TerrainMesh& mesh = tile.mesh;
        if (!mesh.vertices || mesh.vertexCount == 0)
            continue;

        g_vertices.reserve(g_vertices.size() + mesh.vertexCount * 8);

        for (uint32_t v = 0; v < mesh.vertexCount; ++v)
        {
            const TerrainVertex& vert = mesh.vertices[v];
            g_vertices.push_back(vert.x);
            g_vertices.push_back(vert.y);
            g_vertices.push_back(vert.z);
            g_vertices.push_back(vert.u);
            g_vertices.push_back(vert.v);
            g_vertices.push_back(vert.r);
            g_vertices.push_back(vert.g);
            g_vertices.push_back(vert.b);
        }
    }
}