#pragma once
#include <cstdint>
#include <string>

struct TerrainParams;

struct LoadedHeightmap
{
    uint8_t* samples = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct TerrainVertex
{
    float x;
    float y;
    float z;

    float u;
    float v;

    float r;
    float g;
    float b;
};

struct TerrainMesh
{
    TerrainVertex* vertices = nullptr;
    uint32_t vertexCount = 0;
};

struct TerrainTile
{
    uint32_t tileX = 0;
    uint32_t tileY = 0;
    TerrainMesh mesh;
};

class TerrainGenerator
{
public:
    // TerrainGenerator.h
    static bool GenerateTiles(
        const LoadedHeightmap& heightmap,
        TerrainTile*& outTiles,
        uint32_t& outTileCount,
        const TerrainParams& params);

    static LoadedHeightmap LoadHeightmap(const std::string& path);

    static void FreeTiles(
        TerrainTile* tiles,
        uint32_t tileCount
    );
};