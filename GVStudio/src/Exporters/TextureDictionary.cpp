#include "Exporters/TextureDictionary.h"
#include "GVFramework/Chunk/Chunk.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "3rdParty/STB/stb_image.h"

static bool IsAbsolutePath(const std::string& path)
{
    if (path.size() > 2 && path[1] == ':') return true;
    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) return true;
    return false;
}

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
        4
    );

    if (!data)
    {
        std::cerr << "[Image] Failed to load: " << path << "\n";
        std::cerr << "Reason: " << stbi_failure_reason() << "\n";
        return false;
    }

    pixels.resize(width * height * 4);
    memcpy(pixels.data(), data, width * height * 4);

    stbi_image_free(data);

    std::cout << "[Image] Loaded: " << path
        << " (" << width << "x" << height << ")\n";

    return true;
}

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

void GV_Exporter<TextureDictionary>::Build(
    const TextureDictionary&,
    GV_ChunkExporter& out,
    const GV_ExportContext& ctx)
{
    const auto& textures = ctx.GetTextures();

    std::cout << "\n==============================\n";
    std::cout << "[TextureDictionary] Build Start\n";
    std::cout << "Texture Count: " << textures.size() << "\n";

    // Dump all textures + IDs BEFORE processing
    std::cout << "\n[TextureDictionary] Context Dump:\n";
    for (size_t i = 0; i < textures.size(); i++)
    {
        const std::string& name = textures[i];
        uint32_t id = ctx.GetTextureID(name);

        std::cout << "  [" << i << "] "
            << "Name: " << name
            << " | ID: " << id << "\n";
    }
    std::cout << "------------------------------\n";

    for (uint32_t i = 0; i < textures.size(); i++)
    {
        const std::string& name = textures[i];

        std::cout << "\n[Texture] Processing: " << name << "\n";

        int width = 0;
        int height = 0;
        std::vector<uint8_t> rgba;

        std::string fullPath = IsAbsolutePath(name)
            ? name
            : ctx.ResolvePath(name);

        std::cout << "  Resolved Path: " << fullPath << "\n";

        if (!LoadImageRGBA(fullPath, width, height, rgba))
        {
            std::cerr << "[Texture] FAILED LOAD → skipping\n";
            continue;
        }

        bool hasAlpha = HasAlpha(rgba);

        std::cout << "  Alpha: " << (hasAlpha ? "YES" : "NO") << "\n";

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

        GV_ChunkExporter texPayload;

        uint32_t textureID = ctx.GetTextureID(fullPath);
        uint32_t dataSize = (uint32_t)finalPixels.size();

        std::cout << "  FINAL TextureID: " << textureID << "\n";
        std::cout << "  Width: " << width
            << " Height: " << height << "\n";
        std::cout << "  Format: " << format << "\n";
        std::cout << "  DataSize: " << dataSize << "\n";

        texPayload.Write(textureID);
        texPayload.Write((uint32_t)width);
        texPayload.Write((uint32_t)height);
        texPayload.Write(format);
        texPayload.Write(dataSize);

        texPayload.Align16();

        texPayload.WriteData(finalPixels.data(), dataSize);

        std::cout << "  Writing GV_CHUNK_TEXTURE_NATIVE\n";

        out.AppendChunk(
            GV_CHUNK_TEXTURE_NATIVE,
            1,
            texPayload.buffer
        );
    }

    std::cout << "\n[TextureDictionary] Build COMPLETE\n";
    std::cout << "==============================\n\n";
}

uint32_t GV_Exporter<TextureDictionary>::GetChunkType()
{
    return GV_CHUNK_TEXDICTIONARY;
}