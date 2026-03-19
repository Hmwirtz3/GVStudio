#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <string>

#include "GVFramework/Chunk/Chunk.h"
#include "Exporters/ExportContext.h"

class SceneManager;
struct SceneFolder;
class SceneObject;

// ============================================================
// GV_ChunkExporter
// ============================================================

class GV_ChunkExporter
{
public:
    std::vector<char> buffer;

    template<typename T>
    inline void Write(const T& value)
    {
        const char* ptr = reinterpret_cast<const char*>(&value);
        buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
    }

    void WriteData(const void* data, size_t size);
    void Align16();
    void AppendChunk(uint32_t type, uint32_t version, const std::vector<char>& payload);
};


// ============================================================
// Generic Exporter Template
// ============================================================

template<typename T>
struct GV_Exporter
{
    static std::vector<char> Export(const T& asset, const GV_ExportContext& ctx)
    {
        GV_ChunkExporter payloadExporter;
        Build(asset, payloadExporter, ctx);

        GV_ChunkExporter finalExporter;
        finalExporter.AppendChunk(GetChunkType(), GetVersion(), payloadExporter.buffer);

        return finalExporter.buffer;
    }

    static void Build(const T&, GV_ChunkExporter&, const GV_ExportContext&)
    {
        static_assert(sizeof(T) == 0, "GV_Exporter::Build not specialized for this type");
    }

    static uint32_t GetChunkType()
    {
        static_assert(sizeof(T) == 0, "GV_Exporter::GetChunkType not specialized for this type");
        return GV_CHUNK_UNKNOWN;
    }

    static uint32_t GetVersion()
    {
        return 1;
    }
};


// ============================================================
// Entry Point
// ============================================================

bool GV_ExportScene(
    const SceneManager& sceneManager,
    const std::string& outputPath, GV_ExportContext& ctx);