#pragma once

#include <vector>
#include <cstdint>
#include <string>

#include "Exporters/Exporter.h"
#include "Exporters/ExportContext.h"

// ============================================================
// PSP Optimized Vertex
// ============================================================

#pragma pack(push, 1)
struct PSPVertex
{
    
    int16_t u, v;
    uint32_t color;
    float x, y, z;
};
#pragma pack(pop)


// ============================================================
// Editor-side Static Mesh (NO IDs)
// ============================================================

struct StaticMesh
{
    struct Vertex
    {
        float px, py, pz;
        float u, v;
    };

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    std::string meshName;     
    std::string textureName; 
};


// ============================================================
// Exporter Specialization
// ============================================================

template<>
struct GV_Exporter<StaticMesh>
{
    static std::vector<char> Export(const StaticMesh& asset, const GV_ExportContext& ctx)
    {
        GV_ChunkExporter payloadExporter;
        Build(asset, payloadExporter, ctx);

        GV_ChunkExporter finalExporter;
        finalExporter.AppendChunk(GetChunkType(), GetVersion(), payloadExporter.buffer);

        return finalExporter.buffer;
    }

    static void Build(const StaticMesh& asset, GV_ChunkExporter& exporter, const GV_ExportContext& ctx);

    static uint32_t GetChunkType();
    

    static uint32_t GetVersion()
    {
        return 1;
    }
};


// ============================================================
// Conversion Helper
// ============================================================

PSPVertex ConvertToPSPVertex(const StaticMesh::Vertex& v);