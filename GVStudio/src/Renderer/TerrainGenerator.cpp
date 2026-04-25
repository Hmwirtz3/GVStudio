#include "Renderer/TerrainGenerator.h"
#include "Renderer/TerrainManager.h"
#include "3rdParty/STB/stb_image.h"

#include <cstdlib>
#include <string>
#include <cstring>

LoadedHeightmap TerrainGenerator::LoadHeightmap(const std::string& path)
{
    LoadedHeightmap out{};

    int w, h, channels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 1);
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
    const TerrainParams& params)
{
    outTiles = nullptr;
    outTileCount = 0;

    if (!heightmap.samples || heightmap.width == 0 || heightmap.height == 0)
        return false;

    const float sampleSpacing = params.sampleSpacing;
    const float heightScale = params.heightScale;
    const float baseHeight = params.baseHeight;

    const uint32_t tileSize = 128;
    const uint32_t tilesX = (heightmap.width - 1) / tileSize;
    const uint32_t tilesY = (heightmap.height - 1) / tileSize;

    outTileCount = tilesX * tilesY;
    if (outTileCount == 0)
        return false;

    outTiles = (TerrainTile*)std::malloc(sizeof(TerrainTile) * outTileCount);
    if (!outTiles)
        return false;

    uint32_t tileIndex = 0;

    for (uint32_t ty = 0; ty < tilesY; ++ty)
    {
        for (uint32_t tx = 0; tx < tilesX; ++tx)
        {
            TerrainTile& tile = outTiles[tileIndex];
            tile.tileX = tx;
            tile.tileY = ty;

            const uint32_t startX = tx * tileSize;
            const uint32_t startY = ty * tileSize;

            const uint32_t quadCount = tileSize * tileSize;
            const uint32_t vertexCount = quadCount * 6;

            float* verts = (float*)std::malloc(sizeof(float) * vertexCount * 8);
            if (!verts)
            {
                tile.mesh.vertices = nullptr;
                tile.mesh.vertexCount = 0;
                ++tileIndex;
                continue;
            }

            const float worldX = (float)tx * tileSize * sampleSpacing;
            const float worldZ = (float)ty * tileSize * sampleSpacing;

            uint32_t v = 0;

            for (uint32_t y = 0; y < tileSize; ++y)
            {
                for (uint32_t x = 0; x < tileSize; ++x)
                {
                    uint32_t gx0 = startX + x;
                    uint32_t gy0 = startY + y;
                    uint32_t gx1 = gx0 + 1;
                    uint32_t gy1 = gy0 + 1;

                    float s00 = 1.0f - (heightmap.samples[gy0 * heightmap.width + gx0] / 255.0f);
                    float s10 = 1.0f - (heightmap.samples[gy0 * heightmap.width + gx1] / 255.0f);
                    float s01 = 1.0f - (heightmap.samples[gy1 * heightmap.width + gx0] / 255.0f);
                    float s11 = 1.0f - (heightmap.samples[gy1 * heightmap.width + gx1] / 255.0f);

                    float h00 = s00 * heightScale + baseHeight;
                    float h10 = s10 * heightScale + baseHeight;
                    float h01 = s01 * heightScale + baseHeight;
                    float h11 = s11 * heightScale + baseHeight;

                    float x0 = worldX + (float)x * sampleSpacing;
                    float x1 = worldX + (float)(x + 1) * sampleSpacing;
                    float z0 = worldZ + (float)y * sampleSpacing;
                    float z1 = worldZ + (float)(y + 1) * sampleSpacing;

                    float tri[] =
                    {
                        x0, h00, z0,  0,0,  0.3f,0.3f,0.3f,
                        x1, h10, z0,  0,0,  0.3f,0.3f,0.3f,
                        x1, h11, z1,  0,0,  0.3f,0.3f,0.3f,

                        x0, h00, z0,  0,0,  0.3f,0.3f,0.3f,
                        x1, h11, z1,  0,0,  0.3f,0.3f,0.3f,
                        x0, h01, z1,  0,0,  0.3f,0.3f,0.3f
                    };

                    std::memcpy(&verts[v], tri, sizeof(tri));
                    v += 48;
                }
            }

            tile.mesh.vertices = (TerrainVertex*)verts;
            tile.mesh.vertexCount = vertexCount;

            ++tileIndex;
        }
    }

    return true;
}

void TerrainGenerator::FreeTiles(
    TerrainTile* tiles,
    uint32_t tileCount)
{
    if (!tiles)
        return;

    for (uint32_t i = 0; i < tileCount; ++i)
    {
        if (tiles[i].mesh.vertices)
        {
            std::free(tiles[i].mesh.vertices);
            tiles[i].mesh.vertices = nullptr;
        }
    }

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