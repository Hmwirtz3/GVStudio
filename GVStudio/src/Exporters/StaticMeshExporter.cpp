#include "Exporters/StaticMeshExporter.h"
#include "GVFramework/Chunk/Chunk.h"

#include <iostream>

// ============================================================
// Conversion (GV_Vertex → PSPVertex)
// ============================================================

static PSPVertex ConvertToPSPVertex(const GV_Vertex& v)
{
    PSPVertex out{};

    const float scale = 32.0f;

    out.x = (int16_t)(v.x * scale);
    out.y = (int16_t)(v.y * scale);
    out.z = (int16_t)(v.z * scale);

    out.u = (int16_t)(v.u * 32767.0f);
    out.v = (int16_t)(v.v * 32767.0f);

    out.color = 0xFFFFFFFF;

    return out;
}

// ============================================================
// Exporter Implementation (NO MeshID, Submesh-Based)
// ============================================================

void GV_Exporter<StaticMesh>::Build(
    const StaticMesh& mesh,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx)
{
    std::cout << "\n[StaticMesh Export]\n";
    std::cout << "  Mesh: " << mesh.meshName << "\n";

    const GV_MeshData* meshData = ctx.GetMeshData(mesh.meshName);

    if (!meshData)
    {
        std::cout << "  [ERROR] MeshData not found\n";
        return;
    }

    std::cout << "  Submeshes: " << meshData->submeshes.size() << "\n";

    // ========================================================
    // LOOP SUBMESHES
    // ========================================================

    for (size_t i = 0; i < meshData->submeshes.size(); i++)
    {
        const GV_Submesh& sm = meshData->submeshes[i];

        std::cout << "  [Submesh " << i << "]\n";
        std::cout << "    Vertices: " << sm.vertices.size() << "\n";
        std::cout << "    Indices:  " << sm.indices.size() << "\n";
        std::cout << "    TextureID: " << sm.textureID << "\n";

        // ====================================================
        // STRUCT (NO MeshID)
        // ====================================================

        {
            GV_ChunkExporter sub;

            uint32_t submeshIndex = (uint32_t)i;
            uint32_t vertexCount = (uint32_t)sm.vertices.size();
            uint32_t indexCount = (uint32_t)sm.indices.size();
            uint32_t format = 0;

            sub.Write(submeshIndex);
            sub.Write(vertexCount);
            sub.Write(indexCount);
            sub.Write(format);

            sub.Align16();

            exporter.AppendChunk(GV_CHUNK_STRUCT, 1, sub.buffer);
        }

        // ====================================================
        // GEOMETRY
        // ====================================================

        {
            GV_ChunkExporter sub;

            std::vector<PSPVertex> pspVerts;
            pspVerts.reserve(sm.vertices.size());

            for (const auto& v : sm.vertices)
                pspVerts.push_back(ConvertToPSPVertex(v));

            if (!pspVerts.empty())
            {
                sub.WriteData(
                    pspVerts.data(),
                    pspVerts.size() * sizeof(PSPVertex));
            }

            sub.Align16();

            exporter.AppendChunk(GV_CHUNK_GEOMETRY, 1, sub.buffer);
        }

        // ====================================================
        // INDICES
        // ====================================================

        {
            GV_ChunkExporter sub;

            if (!sm.indices.empty())
            {
                sub.WriteData(
                    sm.indices.data(),
                    sm.indices.size() * sizeof(uint16_t));
            }

            sub.Align16();

            exporter.AppendChunk(GV_CHUNK_BIN_MESH_PLG, 1, sub.buffer);
        }

        // ====================================================
        // MATERIAL (PER SUBMESH)
        // ====================================================

        {
            GV_ChunkExporter sub;

            sub.Write(sm.textureID);

            sub.Align16();

            exporter.AppendChunk(GV_CHUNK_MATERIAL, 1, sub.buffer);
        }
    }

    std::cout << "[StaticMesh Export Complete]\n";
}

// ============================================================
// Chunk Type
// ============================================================

uint32_t GV_Exporter<StaticMesh>::GetChunkType()
{
    return GV_CHUNK_STATIC_MESH;
}