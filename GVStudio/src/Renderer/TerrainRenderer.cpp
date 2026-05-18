#include "Renderer/TerrainRenderer.h"
#include "Renderer/TerrainManager.h"
#include "Renderer/TerrainPaintMap.h"

#include <vector>
#include <cstdint>
#include <cmath>

namespace
{
    static std::vector<float> g_vertices;

    static float g_cameraX = 0.0f;
    static float g_cameraZ = 0.0f;

    static int g_lastStartTileX = -999999;
    static int g_lastStartTileY = -999999;
    static uint32_t g_lastTerrainRevision = 0;

    static bool g_dirty = true;

    static int ClampInt(int value, int minValue, int maxValue)
    {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    }

    static float ClampFloat(float v, float minV, float maxV)
    {
        if (v < minV) return minV;
        if (v > maxV) return maxV;
        return v;
    }

    static void PushVertex(
        float x,
        float y,
        float z,
        float u,
        float v,
        float r,
        float g,
        float b)
    {
        g_vertices.push_back(x);
        g_vertices.push_back(y);
        g_vertices.push_back(z);

        g_vertices.push_back(u);
        g_vertices.push_back(v);

        g_vertices.push_back(r);
        g_vertices.push_back(g);
        g_vertices.push_back(b);
    }

    static float ComputeShade(
        float hC,
        float hX,
        float hY,
        const TerrainParams& params)
    {
        float dhx = hX - hC;
        float dhy = hY - hC;

        float slope =
            (fabsf(dhx) + fabsf(dhy)) /
            params.sampleSpacing;

        float shade =
            1.0f - slope * 0.5f;

        return ClampFloat(
            shade,
            0.3f,
            1.0f
        );
    }

    static void ExpandTile(
        const TerrainTile& tile,
        const TerrainParams& params)
    {
        const TerrainPaintMap& paintMap =
            TerrainPaintMap::GetActive();

        const uint32_t vertsPerTileSide = 33;
        const uint32_t quadsPerTile = 32;

        const float tileWorldSize =
            (float)quadsPerTile *
            params.sampleSpacing;

        const float baseX =
            (float)tile.tileX *
            tileWorldSize;

        const float baseZ =
            (float)tile.tileY *
            tileWorldSize;

        for (uint32_t y = 0;
            y < quadsPerTile;
            ++y)
        {
            for (uint32_t x = 0;
                x < quadsPerTile;
                ++x)
            {
                const uint32_t i00 =
                    y * vertsPerTileSide + x;

                const uint32_t i10 =
                    y * vertsPerTileSide + (x + 1);

                const uint32_t i01 =
                    (y + 1) * vertsPerTileSide + x;

                const uint32_t i11 =
                    (y + 1) * vertsPerTileSide + (x + 1);

                const float h00 =
                    ((float)tile.heights[i00] / 255.0f - 0.5f) *
                    params.heightScale +
                    params.baseHeight;

                const float h10 =
                    ((float)tile.heights[i10] / 255.0f - 0.5f) *
                    params.heightScale +
                    params.baseHeight;

                const float h01 =
                    ((float)tile.heights[i01] / 255.0f - 0.5f) *
                    params.heightScale +
                    params.baseHeight;

                const float h11 =
                    ((float)tile.heights[i11] / 255.0f - 0.5f) *
                    params.heightScale +
                    params.baseHeight;

                const float x0 =
                    baseX +
                    (float)x *
                    params.sampleSpacing;

                const float x1 =
                    baseX +
                    (float)(x + 1) *
                    params.sampleSpacing;

                const float z0 =
                    baseZ +
                    (float)y *
                    params.sampleSpacing;

                const float z1 =
                    baseZ +
                    (float)(y + 1) *
                    params.sampleSpacing;

                const int globalX =
                    (int)tile.tileX *
                    (int)quadsPerTile +
                    (int)x;

                const int globalY =
                    (int)tile.tileY *
                    (int)quadsPerTile +
                    (int)y;

                const uint8_t materialID =
                    paintMap.GetCell(
                        globalX,
                        globalY
                    );

                const TerrainMaterialDef* mat =
                    paintMap.GetMaterial(
                        materialID
                    );

                float u0;
                float u1;
                float v0;
                float v1;

                if (mat &&
                    paintMap.GetAtlasTilesX() > 0 &&
                    paintMap.GetAtlasTilesY() > 0)
                {
                    const float atlasTileW =
                        1.0f /
                        (float)paintMap.GetAtlasTilesX();

                    const float atlasTileH =
                        1.0f /
                        (float)paintMap.GetAtlasTilesY();

                    const float padding =
                        0.001f;

                    float localU0 =
                        (0.0f * mat->uvScaleX) +
                        mat->uvOffsetX;

                    float localV0 =
                        (0.0f * mat->uvScaleY) +
                        mat->uvOffsetY;

                    float localU1 =
                        (1.0f * mat->uvScaleX) +
                        mat->uvOffsetX;

                    float localV1 =
                        (1.0f * mat->uvScaleY) +
                        mat->uvOffsetY;

                    localU0 =
                        ClampFloat(
                            localU0,
                            padding,
                            1.0f - padding
                        );

                    localV0 =
                        ClampFloat(
                            localV0,
                            padding,
                            1.0f - padding
                        );

                    localU1 =
                        ClampFloat(
                            localU1,
                            padding,
                            1.0f - padding
                        );

                    localV1 =
                        ClampFloat(
                            localV1,
                            padding,
                            1.0f - padding
                        );

                    u0 =
                        ((float)mat->atlasX + localU0) *
                        atlasTileW;

                    v0 =
                        ((float)mat->atlasY + localV0) *
                        atlasTileH;

                    u1 =
                        ((float)mat->atlasX + localU1) *
                        atlasTileW;

                    v1 =
                        ((float)mat->atlasY + localV1) *
                        atlasTileH;
                }
                else
                {
                    const float uvScale =
                        1.0f / 512.0f;

                    u0 = x0 * uvScale;
                    u1 = x1 * uvScale;

                    v0 = z0 * uvScale;
                    v1 = z1 * uvScale;
                }

                float s00 =
                    ComputeShade(
                        h00,
                        h10,
                        h01,
                        params
                    );

                float s10 =
                    ComputeShade(
                        h10,
                        h11,
                        h00,
                        params
                    );

                float s01 =
                    ComputeShade(
                        h01,
                        h00,
                        h11,
                        params
                    );

                float s11 =
                    ComputeShade(
                        h11,
                        h10,
                        h01,
                        params
                    );

                PushVertex(
                    x0,
                    h00,
                    z0,
                    u0,
                    v0,
                    s00,
                    s00,
                    s00
                );

                PushVertex(
                    x1,
                    h10,
                    z0,
                    u1,
                    v0,
                    s10,
                    s10,
                    s10
                );

                PushVertex(
                    x1,
                    h11,
                    z1,
                    u1,
                    v1,
                    s11,
                    s11,
                    s11
                );

                PushVertex(
                    x0,
                    h00,
                    z0,
                    u0,
                    v0,
                    s00,
                    s00,
                    s00
                );

                PushVertex(
                    x1,
                    h11,
                    z1,
                    u1,
                    v1,
                    s11,
                    s11,
                    s11
                );

                PushVertex(
                    x0,
                    h01,
                    z1,
                    u0,
                    v1,
                    s01,
                    s01,
                    s01
                );
            }
        }
    }
}

void TerrainRenderer::SetCameraPosition(
    float x,
    float z)
{
    g_cameraX = x;
    g_cameraZ = z;

    g_dirty = true;
}

void TerrainRenderer::Build()
{
    if (!TerrainManager::HasTerrain())
    {
        g_vertices.clear();

        g_lastStartTileX = -999999;
        g_lastStartTileY = -999999;

        g_lastTerrainRevision =
            TerrainManager::GetRevision();

        g_dirty = true;

        return;
    }

    const TerrainParams& params =
        TerrainManager::GetParams();

    const float tileWorldSize =
        32.0f * params.sampleSpacing;

    int cameraTileX =
        (int)std::floor(
            g_cameraX /
            tileWorldSize
        );

    int cameraTileY =
        (int)std::floor(
            g_cameraZ /
            tileWorldSize
        );

    const int visibleTiles = 25;

    const int halfVisibleTiles =
        visibleTiles / 2;

    int maxStartX =
        (int)params.tilesX -
        visibleTiles;

    int maxStartY =
        (int)params.tilesY -
        visibleTiles;

    if (maxStartX < 0)
        maxStartX = 0;

    if (maxStartY < 0)
        maxStartY = 0;

    int startTileX =
        ClampInt(
            cameraTileX -
            halfVisibleTiles,
            0,
            maxStartX
        );

    int startTileY =
        ClampInt(
            cameraTileY -
            halfVisibleTiles,
            0,
            maxStartY
        );

    uint32_t revision =
        TerrainManager::GetRevision();

    if (!g_dirty &&
        g_lastStartTileX == startTileX &&
        g_lastStartTileY == startTileY &&
        g_lastTerrainRevision == revision)
    {
        return;
    }

    g_lastStartTileX =
        startTileX;

    g_lastStartTileY =
        startTileY;

    g_lastTerrainRevision =
        revision;

    g_dirty = false;

    RebuildVisibleWindow();
}

void TerrainRenderer::MarkDirty()
{
    g_dirty = true;
}

void TerrainRenderer::RebuildVisibleWindow()
{
    g_vertices.clear();

    if (!TerrainManager::HasTerrain())
        return;

    const TerrainTile* tiles =
        TerrainManager::GetTilesConst();

    const TerrainParams& params =
        TerrainManager::GetParams();

    if (!tiles ||
        params.tilesX == 0 ||
        params.tilesY == 0)
    {
        return;
    }

    const int visibleTiles = 25;

    const int startTileX =
        g_lastStartTileX;

    const int startTileY =
        g_lastStartTileY;

    const int endTileX =
        ClampInt(
            startTileX + visibleTiles,
            0,
            (int)params.tilesX
        );

    const int endTileY =
        ClampInt(
            startTileY + visibleTiles,
            0,
            (int)params.tilesY
        );

    const uint32_t vertsPerTile =
        32 * 32 * 6;

    const uint32_t floatsPerVertex =
        8;

    g_vertices.reserve(
        (endTileX - startTileX) *
        (endTileY - startTileY) *
        vertsPerTile *
        floatsPerVertex
    );

    for (int ty = startTileY;
        ty < endTileY;
        ++ty)
    {
        for (int tx = startTileX;
            tx < endTileX;
            ++tx)
        {
            const uint32_t tileIndex =
                (uint32_t)ty *
                params.tilesX +
                (uint32_t)tx;

            ExpandTile(
                tiles[tileIndex],
                params
            );
        }
    }
}

const std::vector<float>&
TerrainRenderer::GetVertices()
{
    return g_vertices;
}