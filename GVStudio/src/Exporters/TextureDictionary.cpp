#include "Exporters/TextureDictionary.h"
#include "GVFramework/Chunk/Chunk.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

// ============================================================
// Convert RGBA → RGB565
// ============================================================

static uint16_t ConvertRGBAto565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((b >> 3) << 11) |
        ((g >> 2) << 5) |
        ((r >> 3));
}

// ============================================================
// PATH HELPERS
// ============================================================

static bool IsAbsolutePath(const std::string& path)
{
    if (path.size() > 2 && path[1] == ':') return true;
    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) return true;
    return false;
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
// BMP Loader (4bpp + 8bpp)
// ============================================================

static bool LoadImageRGBA(
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

    if (info.biCompression != 0)
    {
        std::cerr << "[BMP] Compressed BMP not supported: " << path << "\n";
        return false;
    }

    width = info.biWidth;
    height = info.biHeight;

    std::cout << "[BMP] w=" << width
        << " h=" << height
        << " bpp=" << info.biBitCount << "\n";

    pixels.resize(width * height * 4);

    // ========================================================
    // 4-bit / 8-bit (PALETTE)
    // ========================================================

    if (info.biBitCount == 8 || info.biBitCount == 4)
    {
        struct PaletteEntry { uint8_t b, g, r, a; };

        int paletteSize = (info.biBitCount == 8) ? 256 : 16;

        std::vector<PaletteEntry> palette(paletteSize);
        file.read((char*)palette.data(), paletteSize * sizeof(PaletteEntry));

        int rowPadded = (info.biBitCount == 8)
            ? (width + 3) & (~3)
            : ((width + 1) / 2 + 3) & (~3);

        std::vector<uint8_t> row(rowPadded);

        file.seekg(header.bfOffBits, std::ios::beg);

        for (int y = 0; y < height; y++)
        {
            file.read((char*)row.data(), rowPadded);

            for (int x = 0; x < width; x++)
            {
                uint8_t index = 0;

                if (info.biBitCount == 8)
                {
                    index = row[x];
                }
                else
                {
                    uint8_t byte = row[x / 2];
                    index = (x % 2 == 0)
                        ? (byte >> 4) & 0x0F
                        : byte & 0x0F;
                }

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

    // ========================================================
    // 🔥 24-BIT SUPPORT (THIS IS THE FIX)
    // ========================================================

    if (info.biBitCount == 24)
    {
        int rowPadded = (width * 3 + 3) & (~3);
        std::vector<uint8_t> row(rowPadded);

        file.seekg(header.bfOffBits, std::ios::beg);

        for (int y = 0; y < height; y++)
        {
            file.read((char*)row.data(), rowPadded);

            for (int x = 0; x < width; x++)
            {
                uint8_t b = row[x * 3 + 0];
                uint8_t g = row[x * 3 + 1];
                uint8_t r = row[x * 3 + 2];

                int dst = ((height - 1 - y) * width + x) * 4;

                pixels[dst + 0] = r;
                pixels[dst + 1] = g;
                pixels[dst + 2] = b;
                pixels[dst + 3] = 255;
            }
        }

        return true;
    }

    // ========================================================
    // UNSUPPORTED
    // ========================================================

    std::cerr << "[BMP] Unsupported BPP: " << info.biBitCount << "\n";
    return false;
}

// ============================================================
// Exporter Implementation
// ============================================================

void GV_Exporter<TextureDictionary>::Build(
    const TextureDictionary&,
    GV_ChunkExporter& out,
    const GV_ExportContext& ctx)
{
    const auto& textures = ctx.GetTextures();

    std::cout << "\n[TextureDictionary] Count: " << textures.size() << "\n";

    for (uint32_t i = 0; i < textures.size(); i++)
    {
        const std::string& name = textures[i];

        int width = 0;
        int height = 0;
        std::vector<uint8_t> rgba;

        std::string fullPath = IsAbsolutePath(name)
            ? name
            : ctx.ResolvePath(name);

        if (!LoadImageRGBA(fullPath, width, height, rgba))
        {
            std::cerr << "[Texture] Failed: " << fullPath << "\n";
            continue;
        }

        // Convert → RGB565
        std::vector<uint16_t> pixels565(width * height);

        for (int p = 0; p < width * height; p++)
        {
            uint8_t r = rgba[p * 4 + 0];
            uint8_t g = rgba[p * 4 + 1];
            uint8_t b = rgba[p * 4 + 2];

            pixels565[p] = ConvertRGBAto565(r, g, b);
        }

        // ----------------------------------------
        // BUILD TEXTURE CHUNK PAYLOAD
        // ----------------------------------------

        GV_ChunkExporter texPayload;

        uint32_t textureID = i + 1 ;
        uint32_t format = 0; // RGB565
        uint32_t dataSize = (uint32_t)(pixels565.size() * sizeof(uint16_t));

        texPayload.Write(textureID);
        texPayload.Write((uint32_t)width);
        texPayload.Write((uint32_t)height);
        texPayload.Write(format);
        texPayload.Write(dataSize);
        texPayload.WriteData(pixels565.data(), dataSize);

        // ----------------------------------------
        // APPEND AS CHILD CHUNK
        // ----------------------------------------

        out.AppendChunk(
            GV_CHUNK_TEXTURE_NATIVE,
            1,
            texPayload.buffer
        );
    }
}

// ============================================================
// Chunk Type
// ============================================================

uint32_t GV_Exporter<TextureDictionary>::GetChunkType()
{
    return GV_CHUNK_TEXDICTIONARY;
}