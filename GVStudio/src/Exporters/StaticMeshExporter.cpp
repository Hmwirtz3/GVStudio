#include "Exporters/StaticMeshExporter.h"
#include "GVFramework/Chunk/Chunk.h"

#include <iostream>
#include <algorithm>



static PSPVertex ConvertToPSPVertex(const GV_Vertex& v)
{
    PSPVertex out{};

    

    out.u = (int16_t)(v.u * 32767.0f);
    out.v = (int16_t)(v.v * 32767.0f);

    float r = std::clamp(v.r, 0.0f, 1.0f);
    float g = std::clamp(v.g, 0.0f, 1.0f);
    float b = std::clamp(v.b, 0.0f, 1.0f);

    uint8_t R = (uint8_t)(r * 255.0f);
    uint8_t G = (uint8_t)(g * 255.0f);
    uint8_t B = (uint8_t)(b * 255.0f);
    uint8_t A = 255;

    out.color = (A << 24) | (B << 16) | (G << 8) | R;

    

    out.x = v.x;
    out.y = v.y;
    out.z = v.z;

    return out;
}



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
        std::cout << "    Src Vertices: " << sm.vertices.size() << "\n";
        std::cout << "    Src Indices:  " << sm.indices.size() << "\n";
        std::cout << "    TextureID:    " << sm.textureID << "\n";

        // ====================================================
        // BUILD EXPANDED TRIANGLE LIST (CRITICAL CHANGE)
        // ====================================================

        std::vector<PSPVertex> pspVerts;
        pspVerts.reserve(sm.indices.size());

        for (size_t j = 0; j < sm.indices.size(); j++)
        {
            uint16_t idx = sm.indices[j];

            if (idx >= sm.vertices.size())
            {
                std::cout << "    [ERROR] Index out of range: " << idx << "\n";
                continue;
            }

            const GV_Vertex& v = sm.vertices[idx];
            pspVerts.push_back(ConvertToPSPVertex(v));
        }

        uint32_t vertexCount = (uint32_t)pspVerts.size();
        uint32_t primitiveType = 0; // 0 = TRIANGLES
        uint32_t format = 0;

        std::cout << "    Final Vertices: " << vertexCount << "\n";
        std::cout << "    Triangles:      " << (vertexCount / 3) << "\n";

        if (vertexCount % 3 != 0)
        {
            std::cout << "    [WARNING] Vertex count not divisible by 3\n";
        }

        // ====================================================
        // STRUCT (UPDATED: NO INDEX COUNT)
        // ====================================================

        {
            GV_ChunkExporter sub;

            uint32_t submeshIndex = (uint32_t)i;

            sub.Write(submeshIndex);
            sub.Write(vertexCount);
            sub.Write(primitiveType);
            sub.Write(format);

            exporter.AppendChunk(GV_CHUNK_STRUCT, 1, sub.buffer);
        }

        // ====================================================
        // GEOMETRY (NOW FINAL DRAW DATA)
        // ====================================================

        {
            GV_ChunkExporter sub;

            if (!pspVerts.empty())
            {
                sub.WriteData(
                    pspVerts.data(),
                    pspVerts.size() * sizeof(PSPVertex));
            }

            exporter.AppendChunk(GV_CHUNK_GEOMETRY, 1, sub.buffer);
        }

        // ====================================================
        // MATERIAL (PER SUBMESH)
        // ====================================================

        {
            GV_ChunkExporter sub;

            uint32_t textureID = 0;

            if (!sm.texturePath.empty())
            {
                textureID = ctx.GetTextureID(sm.texturePath);
            }

            std::cout << "    Final TextureID: " << textureID << "\n";

            sub.Write(textureID);

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