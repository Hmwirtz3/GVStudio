#include "Renderer/TerrainManager.h"

namespace
{
    TerrainTile* g_tiles = nullptr;
    uint32_t g_tileCount = 0;
    TerrainParams g_params{};
    bool g_hasTerrain = false;
}

void TerrainManager::SetTerrain(
    TerrainTile* tiles,
    uint32_t tileCount,
    const TerrainParams& params)
{
    Clear();

    g_tiles = tiles;
    g_tileCount = tileCount;
    g_params = params;
    g_hasTerrain = (tiles != nullptr && tileCount > 0);
}

void TerrainManager::Clear()
{
    if (g_tiles)
    {
        TerrainGenerator::FreeTiles(g_tiles, g_tileCount);
        g_tiles = nullptr;
    }

    g_tileCount = 0;
    g_hasTerrain = false;
}

bool TerrainManager::HasTerrain()
{
    return g_hasTerrain;
}

TerrainTile* TerrainManager::GetTiles()
{
    return g_tiles;
}

const TerrainTile* TerrainManager::GetTilesConst()
{
    return g_tiles;
}

uint32_t TerrainManager::GetTileCount()
{
    return g_tileCount;
}

const TerrainParams& TerrainManager::GetParams()
{
    return g_params;
}