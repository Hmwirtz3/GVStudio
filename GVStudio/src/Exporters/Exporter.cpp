#include "Exporters/Exporter.h"

#include "GVFramework/Scene/SceneManager.h"
#include "GVFramework/Scene/SceneObject.h"

#include "Exporters/StaticMeshExporter.h"
#include "Exporters/TextureDictionary.h"

#include <fstream>
#include <iostream>
#include <algorithm>



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
    Align16();

    GV_ChunkHeader header{};
    header.type = type;
    header.size = sizeof(GV_ChunkHeader) + static_cast<uint32_t>(payload.size());
    header.version = version;

    std::cout << "\n[Exporter] AppendChunk\n";
    std::cout << "  Type: " << type << "\n";
    std::cout << "  Size: " << header.size << "\n";

    Write(header);
    Align16();

    if (!payload.empty())
        WriteData(payload.data(), payload.size());

    Align16();
}



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



bool GV_ExportScene(
    const SceneManager& sceneManager,
    const std::string& outputPath,
    GV_ExportContext& ctx)
{
    std::cout << "\n========== EXPORT START ==========\n";
    std::cout << "[Exporter] Output: " << outputPath << "\n";

    std::cout << "\n[Exporter] Building context...\n";
    BuildContextFolder(sceneManager.GetRootFolder(), ctx);

    const auto& textures = ctx.GetTextures();
    const auto& meshes = ctx.GetMeshes();

    std::cout << "[Exporter] Context built\n";
    std::cout << "  Textures: " << textures.size() << "\n";
    std::cout << "  Meshes:   " << meshes.size() << "\n";

    GV_ChunkExporter scenePayload;

    {
        std::cout << "\n[Scene] Writing Texture Dictionary\n";

        TextureDictionary dict;
        auto data = GV_Exporter<TextureDictionary>::Export(dict, ctx);

        scenePayload.WriteData(data.data(), data.size());
        scenePayload.Align16();
    }

    ExportFolder(sceneManager.GetRootFolder(), scenePayload, ctx);

    GV_ChunkExporter finalExporter;

    finalExporter.AppendChunk(
        GV_CHUNK_SCENE,
        1,
        scenePayload.buffer
    );

    std::cout << "[Exporter] Final size: "
        << finalExporter.buffer.size() << " bytes\n";

    std::ofstream out(outputPath, std::ios::binary);
    if (!out)
    {
        std::cout << "[Exporter] ERROR: failed to open file\n";
        return false;
    }

    out.write(finalExporter.buffer.data(), finalExporter.buffer.size());

    std::cout << "[Exporter] Export complete\n";
    std::cout << "========== EXPORT END ==========\n\n";

    return true;
}


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

                auto extPos = asset.find_last_of('.');
                if (extPos == std::string::npos)
                    continue;

                std::string ext = asset.substr(extPos + 1);
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                if (ext == "bmp" || ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "tga")
{
    ctx.RegisterTexture(asset);
}
                else if (ext == "obj")
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

    GV_ChunkExporter objPayload;
    GV_ChunkExporter innerPayload;

    struct ParamPair
    {
        const LU_Param_Def* def;
        const LU_Param_Val* val;
    };

    std::vector<ParamPair> runtimeParams;

    size_t total = std::min(lu.params.size(), obj.def->values.size());

    for (size_t i = 0; i < total; i++)
    {
        const LU_Param_Def& def = lu.params[i];
        const LU_Param_Val& val = obj.def->values[i];

        if (def.type == ParamType::Separator)
            continue;

        if (def.type == ParamType::Asset)
            continue;

        runtimeParams.push_back({ &def, &val });
    }

    uint32_t paramCount = static_cast<uint32_t>(runtimeParams.size());

    std::cout << "  Writing Runtime Params: " << paramCount << "\n";

    innerPayload.WriteData(&paramCount, sizeof(uint32_t));

    for (const auto& p : runtimeParams)
    {
        switch (p.def->type)
        {
        case ParamType::Float:
            innerPayload.WriteData(&p.val->fval, sizeof(float));
            break;

        case ParamType::Int:
            innerPayload.WriteData(&p.val->ival, sizeof(int));
            break;

        case ParamType::Bool:
            innerPayload.WriteData(&p.val->bval, sizeof(bool));
            break;

        case ParamType::String:
        case ParamType::Event:
        case ParamType::Message:
        {
            uint32_t len = static_cast<uint32_t>(p.val->sval.size());
            innerPayload.WriteData(&len, sizeof(uint32_t));

            if (len > 0)
                innerPayload.WriteData(p.val->sval.data(), len);
            break;
        }

        default:
            break;
        }
    }

    innerPayload.Align16();

    switch (lu.chunkType)
    {
    case GV_CHUNK_STATIC_MESH:
    {
        std::cout << "  → StaticMesh\n";

        StaticMesh mesh;

        for (size_t i = 0; i < total; i++)
        {
            const LU_Param_Def& def = lu.params[i];
            const LU_Param_Val& val = obj.def->values[i];

            if (def.name == "modelPath")
                mesh.meshName = val.sval;

            if (def.name == "texture")
                mesh.textureName = val.sval;
        }

        if (mesh.meshName.empty())
        {
            std::cout << "  [SKIP] No mesh assigned\n";
            return;
        }

        auto data = GV_Exporter<StaticMesh>::Export(mesh, ctx);

        std::cout << "  Size: " << data.size() << " bytes\n";

        innerPayload.WriteData(data.data(), data.size());
        break;
    }

    case GV_CHUNK_TEXTURE:
    {
        std::cout << "  → UI Texture\n";

        std::string textureName;

        for (size_t i = 0; i < total; i++)
        {
            const LU_Param_Def& def = lu.params[i];
            const LU_Param_Val& val = obj.def->values[i];

            if (def.name == "texture")
                textureName = val.sval;
        }

        if (textureName.empty())
        {
            std::cout << "  [SKIP] No texture assigned\n";
            return;
        }

        uint32_t textureID = ctx.GetTextureID(textureName);

        std::cout << "  TextureID: " << textureID << "\n";

        innerPayload.WriteData(&textureID, sizeof(uint32_t));
        break;
    }

    default:
    {
        std::cout << "  → No exporter (param-only chunk)\n";
        break;
    }
    }

    objPayload.AppendChunk(
        lu.chunkType,
        1,
        innerPayload.buffer
    );

    exporter.AppendChunk(
        GV_CHUNK_SCENE_OBJECT,
        1,
        objPayload.buffer
    );
}