#include "Exporters/TextureDictionary.h"
#include "GVFramework/Chunk/Chunk.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "3rdParty/STB/stb_image.h"

// ============================================
// PATH HELPERS
// ============================================

static bool IsAbsolutePath(const std::string& path)
{
    if (path.size() > 2 && path[1] == ':') return true;
    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) return true;
    return false;
}

// ============================================
// LOAD IMAGE (ANY FORMAT)
// ============================================

static bool LoadImageRGBA(
    const std::string& path,
    int& width,
    int& height,
    std::vector<uint8_t>& pixels)
{
    int channels = 0;

    unsigned char* data = stbi_load(
        path.c_str(),
        &width,
        &height,
        &channels,
        4 // force RGBA
    );

    if (!data)
    {
        std::cerr << "[Image] Failed to load: " << path << "\n";
        std::cerr << "Reason: " << stbi_failure_reason() << "\n";
        return false;
    }

    // allocate output
    pixels.resize(width * height * 4);

    const int rowSize = width * 4;

    // flip vertically
    for (int y = 0; y < height; y++)
    {
        const uint8_t* srcRow = data + ((height - 1 - y) * rowSize);
        uint8_t* dstRow = pixels.data() + (y * rowSize);

        memcpy(dstRow, srcRow, rowSize);
    }

    stbi_image_free(data);

    std::cout << "[Image] Loaded (flipped): " << path
        << " (" << width << "x" << height << ")\n";

    return true;
}

// ============================================
// FORMAT HELPERS
// ============================================

enum TextureFormat
{
    TEX_RGB565 = 0,
    TEX_RGBA4444 = 1
};

static bool HasAlpha(const std::vector<uint8_t>& rgba)
{
    for (size_t i = 0; i < rgba.size(); i += 4)
    {
        if (rgba[i + 3] < 255)
            return true;
    }
    return false;
}

static uint16_t To565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((b >> 3) << 11) |
        ((g >> 2) << 5) |
        ((r >> 3));
}

static uint16_t To4444(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return ((a >> 4) << 12) |
        ((b >> 4) << 8) |
        ((g >> 4) << 4) |
        ((r >> 4));
}

// ============================================
// EXPORTER IMPLEMENTATION
// ============================================

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

        bool hasAlpha = HasAlpha(rgba);

        uint32_t format = TEX_RGB565;
        std::vector<char> finalPixels;

        if (hasAlpha)
        {
            format = TEX_RGBA4444;

            std::vector<uint16_t> pixels(width * height);

            for (int p = 0; p < width * height; p++)
            {
                uint8_t r = rgba[p * 4 + 0];
                uint8_t g = rgba[p * 4 + 1];
                uint8_t b = rgba[p * 4 + 2];
                uint8_t a = rgba[p * 4 + 3];

                pixels[p] = To4444(r, g, b, a);
            }

            finalPixels.resize(pixels.size() * sizeof(uint16_t));
            memcpy(finalPixels.data(), pixels.data(), finalPixels.size());
        }
        else
        {
            format = TEX_RGB565;

            std::vector<uint16_t> pixels(width * height);

            for (int p = 0; p < width * height; p++)
            {
                uint8_t r = rgba[p * 4 + 0];
                uint8_t g = rgba[p * 4 + 1];
                uint8_t b = rgba[p * 4 + 2];

                pixels[p] = To565(r, g, b);
            }

            finalPixels.resize(pixels.size() * sizeof(uint16_t));
            memcpy(finalPixels.data(), pixels.data(), finalPixels.size());
        }

        // ----------------------------------------
        // BUILD TEXTURE CHUNK PAYLOAD
        // ----------------------------------------

        GV_ChunkExporter texPayload;

        uint32_t textureID = i + 1;
        uint32_t dataSize = (uint32_t)finalPixels.size();

        texPayload.Write(textureID);
        texPayload.Write((uint32_t)width);
        texPayload.Write((uint32_t)height);
        texPayload.Write(format);
        texPayload.Write(dataSize);

        // ✅ ONLY CHANGE: ALIGN BEFORE PIXEL DATA
        texPayload.Align16();

        texPayload.WriteData(finalPixels.data(), dataSize);

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

// ============================================
// CHUNK TYPE
// ============================================

uint32_t GV_Exporter<TextureDictionary>::GetChunkType()
{
    return GV_CHUNK_TEXDICTIONARY;
}