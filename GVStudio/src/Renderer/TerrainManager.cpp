#include "Renderer/TerrainManager.h"
#include "Renderer/TerrainGenerator.h"

namespace
{
    TerrainTile* g_tiles = nullptr;
    uint32_t g_tileCount = 0;
    TerrainParams g_params{};
    bool g_hasTerrain = false;
    uint32_t g_revision = 0;
}

void TerrainManager::SetTerrain(
    TerrainTile* tiles,
    uint32_t tileCount,
    const TerrainParams& params)
{
    Clear();

    g_params = params;

    if (tiles && tileCount > 0)
    {
        g_tiles = tiles;
        g_tileCount = tileCount;
        g_hasTerrain = true;
        ++g_revision;
        return;
    }

    if (g_params.heightmapPath.empty())
    {
        g_hasTerrain = false;
        ++g_revision;
        return;
    }

    LoadedHeightmap hm = TerrainGenerator::LoadHeightmap(g_params.heightmapPath);

    if (!hm.samples)
    {
        g_hasTerrain = false;
        ++g_revision;
        return;
    }

    TerrainTile* newTiles = nullptr;
    uint32_t newCount = 0;

    if (TerrainGenerator::GenerateTiles(hm, newTiles, newCount, g_params))
    {
        g_tiles = newTiles;
        g_tileCount = newCount;
        g_hasTerrain = true;
    }
    else
    {
        g_hasTerrain = false;
    }

    FreeHeightmap(hm);

    ++g_revision;
}

void TerrainManager::Clear()
{
    if (g_tiles)
    {
        TerrainGenerator::FreeTiles(g_tiles, g_tileCount);
        g_tiles = nullptr;
    }

    g_tileCount = 0;
    g_params = TerrainParams{};
    g_hasTerrain = false;

    ++g_revision;
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

uint32_t TerrainManager::GetRevision()
{
    return g_revision;
}