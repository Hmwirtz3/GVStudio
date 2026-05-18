#include "Renderer/TerrainGenerator.h"
#include "Renderer/TerrainPaintMap.h"

#include "3rdParty/STB/stb_image.h"

#include <cstdlib>
#include <cstring>

static uint8_t SampleHeight(
    const LoadedHeightmap& heightmap,
    uint32_t x,
    uint32_t y)
{
    if (x >= heightmap.width)
        x = heightmap.width - 1;

    if (y >= heightmap.height)
        y = heightmap.height - 1;

    return heightmap.samples[
        y * heightmap.width + x
    ];
}

LoadedHeightmap TerrainGenerator::LoadHeightmap(
    const std::string& path)
{
    LoadedHeightmap out{};

    int w = 0;
    int h = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(false);

    unsigned char* data =
        stbi_load(
            path.c_str(),
            &w,
            &h,
            &channels,
            1
        );

    if (!data)
        return out;

    out.samples = data;
    out.width = (uint32_t)w;
    out.height = (uint32_t)h;

    return out;
}

bool TerrainGenerator::GenerateTiles(
    const LoadedHeightmap& heightmap,
    TerrainTile*& outTiles,
    uint32_t& outTileCount,
    TerrainParams& params)
{
    outTiles = nullptr;
    outTileCount = 0;

    if (!heightmap.samples ||
        heightmap.width < 33 ||
        heightmap.height < 33)
    {
        return false;
    }

    const uint32_t quadsPerTile = 32;
    const uint32_t vertsPerTileSide = 33;

    const uint32_t tilesX =
        (heightmap.width - 1) /
        quadsPerTile;

    const uint32_t tilesY =
        (heightmap.height - 1) /
        quadsPerTile;

    if (tilesX == 0 || tilesY == 0)
        return false;

    outTileCount =
        tilesX * tilesY;

    outTiles =
        (TerrainTile*)std::malloc(
            sizeof(TerrainTile) *
            outTileCount
        );

    if (!outTiles)
    {
        outTileCount = 0;
        return false;
    }

    std::memset(
        outTiles,
        0,
        sizeof(TerrainTile) *
        outTileCount
    );

    params.tilesX = tilesX;
    params.tilesY = tilesY;

    TerrainPaintMap& paintMap =
        TerrainPaintMap::GetActive();

    paintMap.Create(
        tilesX * quadsPerTile,
        tilesY * quadsPerTile
    );

    paintMap.Clear(0);

    uint32_t tileIndex = 0;

    for (uint32_t tileY = 0;
        tileY < tilesY;
        ++tileY)
    {
        for (uint32_t tileX = 0;
            tileX < tilesX;
            ++tileX)
        {
            TerrainTile& tile =
                outTiles[tileIndex];

            tile.tileX = tileX;
            tile.tileY = tileY;

            const uint32_t startX =
                tileX * quadsPerTile;

            const uint32_t startY =
                tileY * quadsPerTile;

            for (uint32_t y = 0;
                y < vertsPerTileSide;
                ++y)
            {
                for (uint32_t x = 0;
                    x < vertsPerTileSide;
                    ++x)
                {
                    const uint32_t hx =
                        startX + x;

                    const uint32_t hy =
                        startY + y;

                    tile.heights[
                        y * vertsPerTileSide + x
                    ] =
                        SampleHeight(
                            heightmap,
                            hx,
                            hy
                        );
                }
            }

            ++tileIndex;
        }
    }

    return true;
}

void TerrainGenerator::FreeTiles(
    TerrainTile* tiles,
    uint32_t tileCount)
{
    (void)tileCount;

    if (!tiles)
        return;

    std::free(tiles);
}

void FreeHeightmap(LoadedHeightmap& hm)
{
    if (hm.samples)
    {
        stbi_image_free(hm.samples);
        hm.samples = nullptr;
    }

    hm.width = 0;
    hm.height = 0;
}