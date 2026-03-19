#include "Exporters/Exporter.h"

#include "GVFramework/Scene/SceneManager.h"
#include "GVFramework/Scene/SceneObject.h"

#include "Exporters/StaticMeshExporter.h"
#include "Exporters/TextureDictionary.h"

#include <fstream>
#include <iostream>
#include <algorithm>

// ============================================================
// GV_ChunkExporter Implementation
// ============================================================

void GV_ChunkExporter::WriteData(const void* data, size_t size)
{
    if (!data || size == 0)
    {
        std::cout << "[Exporter] WriteData skipped\n";
        return;
    }

    std::cout << "[Exporter] WriteData: " << size << " bytes\n";

    const char* ptr = reinterpret_cast<const char*>(data);
    buffer.insert(buffer.end(), ptr, ptr + size);
}

void GV_ChunkExporter::Align16()
{
    size_t before = buffer.size();
    size_t pad = (16 - (buffer.size() % 16)) % 16;

    if (pad)
    {
        buffer.insert(buffer.end(), pad, 0);
        std::cout << "[Exporter] Align16: +" << pad
            << " (" << before << " -> " << buffer.size() << ")\n";
    }
}

void GV_ChunkExporter::AppendChunk(uint32_t type, uint32_t version, const std::vector<char>& payload)
{
    GV_ChunkHeader header{};
    header.type = type;
    header.size = sizeof(GV_ChunkHeader) + static_cast<uint32_t>(payload.size());
    header.version = version;

    std::cout << "\n[Exporter] AppendChunk\n";
    std::cout << "  Type: " << type << "\n";
    std::cout << "  Size: " << header.size << "\n";

    Write(header);

    if (!payload.empty())
        WriteData(payload.data(), payload.size());

    Align16();
}

// ============================================================
// Forward Declarations
// ============================================================

static void BuildContextFolder(
    const SceneFolder& folder,
    GV_ExportContext& ctx);

static void ExportFolder(
    const SceneFolder& folder,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx);

static void ExportObject(
    const SceneObject& obj,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx);

// ============================================================
// Entry Point (FIXED)
// ============================================================

bool GV_ExportScene(
    const SceneManager& sceneManager,
    const std::string& outputPath,
    GV_ExportContext& ctx)
{
    std::cout << "\n========== EXPORT START ==========\n";
    std::cout << "[Exporter] Output: " << outputPath << "\n";

    // PASS 1
    std::cout << "\n[Exporter] Building context...\n";
    BuildContextFolder(sceneManager.GetRootFolder(), ctx);

    const auto& textures = ctx.GetTextures();
    const auto& meshes = ctx.GetMeshes();

    std::cout << "[Exporter] Context built\n";
    std::cout << "  Textures: " << textures.size() << "\n";
    std::cout << "  Meshes:   " << meshes.size() << "\n";

    // PASS 2
    GV_ChunkExporter exporter;

    ExportFolder(sceneManager.GetRootFolder(), exporter, ctx);

    std::cout << "[Exporter] Final size: "
        << exporter.buffer.size() << " bytes\n";

    std::ofstream out(outputPath, std::ios::binary);
    if (!out)
    {
        std::cout << "[Exporter] ERROR: failed to open file\n";
        return false;
    }

    out.write(exporter.buffer.data(), exporter.buffer.size());

    std::cout << "[Exporter] Export complete\n";
    std::cout << "========== EXPORT END ==========\n\n";

    return true;
}

// ============================================================
// PASS 1: CONTEXT BUILD
// ============================================================

static void BuildContextFolder(
    const SceneFolder& folder,
    GV_ExportContext& ctx)
{
    for (const auto& obj : folder.objects)
    {
        if (!obj || !obj->def || !obj->def->def)
            continue;

        const GV_Logic_Unit& lu = *obj->def->def;

        size_t count = std::min(lu.params.size(), obj->def->values.size());

        for (size_t i = 0; i < count; i++)
        {
            const LU_Param_Def& pDef = lu.params[i];
            const LU_Param_Val& val = obj->def->values[i];

            if (pDef.type == ParamType::Asset && !val.sval.empty())
            {
                const std::string& asset = val.sval;

                std::cout << "[Context] " << asset << "\n";

                if (asset.find(".bmp") != std::string::npos)
                {
                    ctx.RegisterTexture(asset);
                }
                else if (asset.find(".obj") != std::string::npos)
                {
                    ctx.RegisterMesh(asset);
                }
            }
        }
    }

    for (const auto& child : folder.children)
    {
        if (child)
            BuildContextFolder(*child, ctx);
    }
}

// ============================================================
// PASS 2: EXPORT
// ============================================================

static void ExportFolder(
    const SceneFolder& folder,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx)
{
    std::cout << "\n[Folder] " << folder.name
        << " | Objects: " << folder.objects.size()
        << " | Children: " << folder.children.size() << "\n";

    for (const auto& obj : folder.objects)
    {
        if (!obj)
        {
            std::cout << "  [Exporter] Null object\n";
            continue;
        }

        ExportObject(*obj, exporter, ctx);
    }

    for (const auto& child : folder.children)
    {
        if (child)
            ExportFolder(*child, exporter, ctx);
    }
}

// ============================================================
// OBJECT EXPORT
// ============================================================

static void ExportObject(
    const SceneObject& obj,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx)
{
    std::cout << "\n[Object] " << obj.name << "\n";

    if (!obj.def || !obj.def->def)
    {
        std::cout << "  No LogicUnit\n";
        return;
    }

    const GV_Logic_Unit& lu = *obj.def->def;

    std::cout << "  LogicUnit: " << lu.typeName << "\n";
    std::cout << "  ChunkType: " << lu.chunkType << "\n";

    switch (lu.chunkType)
    {
    case GV_CHUNK_STATIC_MESH:
    {
        std::cout << "  → StaticMesh\n";

        StaticMesh mesh;

        size_t count = std::min(lu.params.size(), obj.def->values.size());

        for (size_t i = 0; i < count; i++)
        {
            const LU_Param_Def& def = lu.params[i];
            const LU_Param_Val& val = obj.def->values[i];

            if (def.name == "modelPath")
                mesh.meshName = val.sval;

            if (def.name == "texture")
                mesh.textureName = val.sval;
        }

        auto data = GV_Exporter<StaticMesh>::Export(mesh, ctx);

        std::cout << "  Size: " << data.size() << " bytes\n";

        exporter.WriteData(data.data(), data.size());
        break;
    }

    case GV_CHUNK_TEXDICTIONARY:
    {
        std::cout << "  → TextureDictionary\n";

        TextureDictionary dict;

        auto data = GV_Exporter<TextureDictionary>::Export(dict, ctx);

        std::cout << "  Size: " << data.size() << " bytes\n";

        exporter.WriteData(data.data(), data.size());
        break;
    }

    default:
    {
        std::cout << "  → No exporter\n";
        break;
    }
    }
}