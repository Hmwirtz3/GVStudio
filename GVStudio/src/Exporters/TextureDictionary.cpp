#include "Exporters/TextureDictionary.h"
#include "GVFramework/Chunk/Chunk.h"

#include <iostream>
#include <fstream>
#include <vector>

// ============================================================
// Convert RGBA → RGB565
// ============================================================

uint16_t ConvertRGBAto565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r >> 3) << 11) |
        ((g >> 2) << 5) |
        ((b >> 3));
}


// ============================================================
// BMP STRUCTS
// ============================================================

#pragma pack(push, 1)
struct BMPHeader
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BMPInfoHeader
{
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)


// ============================================================
// BMP Loader (8-bit indexed only)
// ============================================================

bool LoadImageRGBA(
    const std::string& path,
    int& width,
    int& height,
    std::vector<uint8_t>& pixels)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "[BMP] Failed to open: " << path << "\n";
        return false;
    }

    BMPHeader header{};
    BMPInfoHeader info{};

    file.read((char*)&header, sizeof(header));
    file.read((char*)&info, sizeof(info));

    if (header.bfType != 0x4D42)
    {
        std::cerr << "[BMP] Invalid file: " << path << "\n";
        return false;
    }

    if (info.biBitCount != 8 || info.biCompression != 0)
    {
        std::cerr << "[BMP] Only 8-bit uncompressed supported: " << path << "\n";
        return false;
    }

    width = info.biWidth;
    height = info.biHeight;

    // ========================================================
    // READ PALETTE (256 entries)
    // ========================================================

    struct PaletteEntry
    {
        uint8_t b, g, r, a;
    };

    PaletteEntry palette[256];
    file.read((char*)palette, sizeof(palette));

    // ========================================================
    // READ PIXELS
    // ========================================================

    int rowPadded = (width + 3) & (~3);
    std::vector<uint8_t> row(rowPadded);

    pixels.resize(width * height * 4);

    file.seekg(header.bfOffBits, std::ios::beg);

    for (int y = 0; y < height; y++)
    {
        file.read((char*)row.data(), rowPadded);

        for (int x = 0; x < width; x++)
        {
            uint8_t index = row[x];
            const auto& p = palette[index];

            int dst = ((height - 1 - y) * width + x) * 4;

            pixels[dst + 0] = p.r;
            pixels[dst + 1] = p.g;
            pixels[dst + 2] = p.b;
            pixels[dst + 3] = 255;
        }
    }

    return true;
}


// ============================================================
// Exporter Implementation
// ============================================================

void GV_Exporter<TextureDictionary>::Build(
    const TextureDictionary&,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx)
{
    const auto& textures = ctx.GetTextures();

    for (uint32_t i = 0; i < textures.size(); i++)
    {
        const std::string& name = textures[i];

        int width = 0;
        int height = 0;
        std::vector<uint8_t> rgba;

        // Use proper path resolution
        std::string fullPath = ctx.ResolvePath(name);

        if (!LoadImageRGBA(fullPath, width, height, rgba))
        {
            std::cerr << "[Texture] Failed to load: " << fullPath << "\n";
            continue;
        }

        // ====================================================
        // Convert to RGB565
        // ====================================================

        std::vector<uint16_t> pixels565;
        pixels565.resize(width * height);

        for (int p = 0; p < width * height; p++)
        {
            uint8_t r = rgba[p * 4 + 0];
            uint8_t g = rgba[p * 4 + 1];
            uint8_t b = rgba[p * 4 + 2];

            pixels565[p] = ConvertRGBAto565(r, g, b);
        }

        // ====================================================
        // TEXTURE CHUNK
        // ====================================================

        GV_ChunkExporter sub;

        uint32_t textureID = i;
        uint32_t format = 0; // RGB565

        sub.Write(textureID);
        sub.Write((uint32_t)width);
        sub.Write((uint32_t)height);
        sub.Write(format);

        sub.WriteData(
            pixels565.data(),
            pixels565.size() * sizeof(uint16_t));

        exporter.AppendChunk(GV_CHUNK_TEXTURE, 1, sub.buffer);
    }
}


// ============================================================
// Chunk Type
// ============================================================

uint32_t GV_Exporter<TextureDictionary>::GetChunkType()
{
    return GV_CHUNK_TEXDICTIONARY;
}