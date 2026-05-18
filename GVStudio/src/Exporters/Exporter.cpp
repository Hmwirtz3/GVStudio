#include "Exporters/Exporter.h"

#include "GVFramework/Scene/SceneManager.h"
#include "GVFramework/Scene/SceneObject.h"

#include "Exporters/StaticMeshExporter.h"
#include "Exporters/TextureDictionary.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cstdio>
#include <cstring>

struct AudioData
{
    std::vector<uint8_t> pcm;
    uint32_t sampleRate = 0;
    uint32_t channels = 0;
};

static bool LoadWav(const std::string& path, AudioData& out)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        std::cout << "[Audio] Failed to open: " << path << "\n";
        return false;
    }

    char riff[4];
    f.read(riff, 4);

    f.seekg(22);
    uint16_t channels;
    f.read((char*)&channels, 2);

    uint32_t sampleRate;
    f.read((char*)&sampleRate, 4);

    f.seekg(34);
    uint16_t bitsPerSample;
    f.read((char*)&bitsPerSample, 2);

    if (bitsPerSample != 16)
    {
        std::cout << "[Audio] Unsupported bit depth\n";
        return false;
    }

    char chunk[4];
    uint32_t size;

    while (f.read(chunk, 4))
    {
        f.read((char*)&size, 4);

        if (std::strncmp(chunk, "data", 4) == 0)
        {
            out.pcm.resize(size);
            f.read((char*)out.pcm.data(), size);

            out.sampleRate = sampleRate;
            out.channels = channels;

            std::cout << "[Audio] Loaded WAV: " << path << "\n";
            std::cout << "  SampleRate: " << sampleRate << "\n";
            std::cout << "  Channels:   " << channels << "\n";
            std::cout << "  Size:       " << size << "\n";

            return true;
        }

        f.seekg(size, std::ios::cur);
    }

    return false;
}

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

        std::cout
            << "[Exporter] Align16: +"
            << pad
            << " ("
            << before
            << " -> "
            << buffer.size()
            << ")\n";
    }
}

void GV_ChunkExporter::AppendChunk(
    uint32_t type,
    uint32_t version,
    const std::vector<char>& payload)
{
    Align16();

    GV_ChunkHeader header{};
    header.type = type;
    header.size =
        sizeof(GV_ChunkHeader)
        + static_cast<uint32_t>(payload.size());

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

static void PreRegisterFolderAssets(
    const SceneFolder& folder,
    GV_ExportContext& ctx);

static void PreRegisterObjectAssets(
    const SceneObject& obj,
    GV_ExportContext& ctx);

static void ExportFolder(
    const SceneFolder& folder,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx,
    std::unordered_map<std::string, int>& instanceCounter);

static void ExportObject(
    const SceneObject& obj,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx,
    std::unordered_map<std::string, int>& instanceCounter);

static void LogTextureIDPSPHex(
    const std::string& textureName,
    uint32_t textureID)
{
    std::cout << "  Texture: " << textureName << "\n";
    std::cout << "  TextureID: " << textureID << "\n";
    std::cout << "  PSP HEX: ";

    std::printf(
        "%02X %02X %02X %02X\n",
        (textureID >> 0) & 0xFF,
        (textureID >> 8) & 0xFF,
        (textureID >> 16) & 0xFF,
        (textureID >> 24) & 0xFF
    );
}

bool GV_ExportScene(
    const SceneManager& sceneManager,
    const std::string& outputPath,
    GV_ExportContext& ctx)
{
    std::cout << "\n========== EXPORT START ==========\n";
    std::cout << "[Exporter] Output: " << outputPath << "\n";

    std::cout << "\n[Scene] Pre-registering assets\n";

    PreRegisterFolderAssets(
        sceneManager.GetRootFolder(),
        ctx
    );

    const auto& textures = ctx.GetTextures();
    const auto& meshes = ctx.GetMeshes();

    std::cout << "[Exporter] Using renderer context\n";
    std::cout << "  Textures: " << textures.size() << "\n";
    std::cout << "  Meshes:   " << meshes.size() << "\n";

    GV_ChunkExporter scenePayload;

    {
        std::cout << "\n[Scene] Writing Texture Dictionary\n";

        TextureDictionary dict;

        auto data =
            GV_Exporter<TextureDictionary>::Export(
                dict,
                ctx
            );

        scenePayload.WriteData(
            data.data(),
            data.size()
        );

        scenePayload.Align16();
    }

    std::unordered_map<std::string, int> instanceCounter;

    ExportFolder(
        sceneManager.GetRootFolder(),
        scenePayload,
        ctx,
        instanceCounter
    );

    GV_ChunkExporter finalExporter;

    finalExporter.AppendChunk(
        GV_CHUNK_SCENE,
        1,
        scenePayload.buffer
    );

    std::cout
        << "[Exporter] Final size: "
        << finalExporter.buffer.size()
        << " bytes\n";

    std::ofstream out(
        outputPath,
        std::ios::binary
    );

    if (!out)
    {
        std::cout
            << "[Exporter] ERROR: failed to open file\n";

        return false;
    }

    out.write(
        finalExporter.buffer.data(),
        finalExporter.buffer.size()
    );

    std::cout << "[Exporter] Export complete\n";
    std::cout << "========== EXPORT END ==========\n\n";

    return true;
}

static void PreRegisterFolderAssets(
    const SceneFolder& folder,
    GV_ExportContext& ctx)
{
    for (const auto& obj : folder.objects)
    {
        if (obj)
            PreRegisterObjectAssets(*obj, ctx);
    }

    for (const auto& child : folder.children)
    {
        if (child)
            PreRegisterFolderAssets(*child, ctx);
    }
}

static void PreRegisterObjectAssets(
    const SceneObject& obj,
    GV_ExportContext& ctx)
{
    if (!obj.def || !obj.def->def)
        return;

    const GV_Logic_Unit& lu = *obj.def->def;

    size_t total =
        std::min(
            lu.params.size(),
            obj.def->values.size()
        );

    if (
        lu.chunkType == GV_CHUNK_BUTTON
        || lu.chunkType == GV_CHUNK_HEIGHTMAP
        || lu.chunkType == GV_CHUNK_CROSSHAIR
        || lu.chunkType == GV_CHUNK_SCROLLING_MAP
        )
    {
        for (size_t i = 0; i < total; i++)
        {
            const LU_Param_Def& def = lu.params[i];
            const LU_Param_Val& val = obj.def->values[i];

            if (def.type != ParamType::Asset)
                continue;

            if (val.sval.empty())
                continue;

            std::string resolved =
                ctx.ResolvePath(val.sval);

            ctx.RegisterTexture(resolved);

            std::cout
                << "    Texture: "
                << val.sval
                << "\n";

            std::cout
                << "    Resolved: "
                << resolved
                << "\n";
        }
    }
}

static void ExportFolder(
    const SceneFolder& folder,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx,
    std::unordered_map<std::string, int>& instanceCounter)
{
    std::cout
        << "\n[Folder] "
        << folder.name
        << " | Objects: "
        << folder.objects.size()
        << " | Children: "
        << folder.children.size()
        << "\n";

    for (const auto& obj : folder.objects)
    {
        if (!obj)
        {
            std::cout << "  [Exporter] Null object\n";
            continue;
        }

        ExportObject(
            *obj,
            exporter,
            ctx,
            instanceCounter
        );
    }

    for (const auto& child : folder.children)
    {
        if (child)
        {
            ExportFolder(
                *child,
                exporter,
                ctx,
                instanceCounter
            );
        }
    }
}

static void ExportObject(
    const SceneObject& obj,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx,
    std::unordered_map<std::string, int>& instanceCounter)
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

    size_t total =
        std::min(
            lu.params.size(),
            obj.def->values.size()
        );

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

    uint32_t paramCount =
        static_cast<uint32_t>(
            runtimeParams.size()
            );

    std::cout
        << "  Writing Runtime Params: "
        << paramCount
        << "\n";

    innerPayload.WriteData(
        &paramCount,
        sizeof(uint32_t)
    );

    for (const auto& p : runtimeParams)
    {
        switch (p.def->type)
        {
        case ParamType::Float:
            innerPayload.WriteData(
                &p.val->fval,
                sizeof(float)
            );
            break;

        case ParamType::Int:
            innerPayload.WriteData(
                &p.val->ival,
                sizeof(int)
            );
            break;

        case ParamType::Bool:
            innerPayload.WriteData(
                &p.val->bval,
                sizeof(bool)
            );
            break;

        case ParamType::String:
        case ParamType::Event:
        case ParamType::Message:
        {
            uint32_t len =
                static_cast<uint32_t>(
                    p.val->sval.size()
                    );

            innerPayload.WriteData(
                &len,
                sizeof(uint32_t)
            );

            if (len > 0)
            {
                innerPayload.WriteData(
                    p.val->sval.data(),
                    len
                );
            }

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

        std::string base = mesh.meshName;

        int& index =
            instanceCounter[base];

        if (index == 0)
            mesh.meshName = base;
        else
            mesh.meshName =
            base
            + "_inst_"
            + std::to_string(index);

        index++;

        auto data =
            GV_Exporter<StaticMesh>::Export(
                mesh,
                ctx
            );

        std::cout
            << "  Size: "
            << data.size()
            << " bytes\n";

        innerPayload.WriteData(
            data.data(),
            data.size()
        );

        break;
    }

    case GV_CHUNK_TEXTURE:
    case GV_CHUNK_SCROLLING_MAP:
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

        std::string resolved =
            ctx.ResolvePath(textureName);

        uint32_t textureID =
            ctx.GetTextureID(resolved);

        LogTextureIDPSPHex(
            textureName,
            textureID
        );

        innerPayload.WriteData(
            &textureID,
            sizeof(uint32_t)
        );

        break;
    }

    case GV_CHUNK_HEIGHTMAP:
    {
        std::cout << "  → Heightmap\n";

        std::string heightmapTexture;

        float sampleSpacing = 128.0f;
        float heightScale = 1.0f;
        float baseHeight = 0.0f;

        for (size_t i = 0; i < total; i++)
        {
            const LU_Param_Def& def = lu.params[i];
            const LU_Param_Val& val = obj.def->values[i];

            if (def.name == "heightmap")
                heightmapTexture = val.sval;

            if (def.name == "sampleSpacing")
                sampleSpacing = val.fval;

            if (def.name == "heightScale")
                heightScale = val.fval;

            if (def.name == "baseHeight")
                baseHeight = val.fval;
        }

        if (heightmapTexture.empty())
        {
            std::cout << "  [SKIP] No heightmap assigned\n";
            return;
        }

        std::string resolved =
            ctx.ResolvePath(heightmapTexture);

        uint32_t textureID =
            ctx.GetTextureID(resolved);

        LogTextureIDPSPHex(
            heightmapTexture,
            textureID
        );

        innerPayload.WriteData(
            &textureID,
            sizeof(uint32_t)
        );

        innerPayload.WriteData(
            &sampleSpacing,
            sizeof(float)
        );

        innerPayload.WriteData(
            &heightScale,
            sizeof(float)
        );

        innerPayload.WriteData(
            &baseHeight,
            sizeof(float)
        );

        break;
    }

    case GV_CHUNK_BUTTON:
    {
        std::cout << "  → Button\n";

        std::string idleTex;
        std::string selectedTex;
        std::string disabledTex;

        for (size_t i = 0; i < total; i++)
        {
            const LU_Param_Def& def = lu.params[i];
            const LU_Param_Val& val = obj.def->values[i];

            if (def.name == "textureIdle")
                idleTex = val.sval;

            if (def.name == "textureSelected")
                selectedTex = val.sval;

            if (def.name == "textureDisabled")
                disabledTex = val.sval;
        }

        uint32_t idleID = 0;
        uint32_t selectedID = 0;
        uint32_t disabledID = 0;

        if (!idleTex.empty())
        {
            std::string resolved =
                ctx.ResolvePath(idleTex);

            idleID =
                ctx.GetTextureID(resolved);

            LogTextureIDPSPHex(
                idleTex,
                idleID
            );
        }

        if (!selectedTex.empty())
        {
            std::string resolved =
                ctx.ResolvePath(selectedTex);

            selectedID =
                ctx.GetTextureID(resolved);

            LogTextureIDPSPHex(
                selectedTex,
                selectedID
            );
        }

        if (!disabledTex.empty())
        {
            std::string resolved =
                ctx.ResolvePath(disabledTex);

            disabledID =
                ctx.GetTextureID(resolved);

            LogTextureIDPSPHex(
                disabledTex,
                disabledID
            );
        }

        innerPayload.WriteData(
            &idleID,
            sizeof(uint32_t)
        );

        innerPayload.WriteData(
            &selectedID,
            sizeof(uint32_t)
        );

        innerPayload.WriteData(
            &disabledID,
            sizeof(uint32_t)
        );

        break;
    }

    case GV_CHUNK_CROSSHAIR:
    {
        std::cout << "  → Crosshair\n";

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

        std::string resolved =
            ctx.ResolvePath(textureName);

        uint32_t textureID =
            ctx.GetTextureID(resolved);

        LogTextureIDPSPHex(
            textureName,
            textureID
        );

        innerPayload.WriteData(
            &textureID,
            sizeof(uint32_t)
        );

        break;
    }

    case GV_CHUNK_AUDIO_SOURCE:
    {
        std::cout << "  → AudioSource\n";

        std::string audioPath;

        for (size_t i = 0; i < total; i++)
        {
            const LU_Param_Def& def = lu.params[i];
            const LU_Param_Val& val = obj.def->values[i];

            if (def.name == "audioFile")
                audioPath = val.sval;
        }

        if (audioPath.empty())
        {
            std::cout << "  [SKIP] No audio assigned\n";
            return;
        }

        std::string resolved =
            ctx.ResolvePath(audioPath);

        std::string ext;

        if (resolved.size() >= 4)
        {
            ext =
                resolved.substr(
                    resolved.size() - 4
                );

            std::transform(
                ext.begin(),
                ext.end(),
                ext.begin(),
                ::tolower
            );
        }

        if (ext == ".mp3")
        {
            std::cout
                << "  Loading MP3: "
                << resolved
                << "\n";

            std::ifstream f(
                resolved,
                std::ios::binary
            );

            if (!f)
            {
                std::cout
                    << "  [ERROR] Failed to open MP3\n";

                return;
            }

            f.seekg(0, std::ios::end);

            uint32_t size =
                (uint32_t)f.tellg();

            f.seekg(0, std::ios::beg);

            std::vector<uint8_t> data(size);

            f.read(
                (char*)data.data(),
                size
            );

            uint32_t sampleRate = 0;
            uint32_t channels = 0;

            innerPayload.WriteData(
                &sampleRate,
                sizeof(uint32_t)
            );

            innerPayload.WriteData(
                &channels,
                sizeof(uint32_t)
            );

            innerPayload.WriteData(
                &size,
                sizeof(uint32_t)
            );

            innerPayload.WriteData(
                data.data(),
                size
            );

            innerPayload.Align16();
        }
        else
        {
            std::cout
                << "  Loading WAV: "
                << resolved
                << "\n";

            AudioData audio;

            if (!LoadWav(resolved, audio))
            {
                std::cout
                    << "  [ERROR] Failed to load WAV\n";

                return;
            }

            innerPayload.WriteData(
                &audio.sampleRate,
                sizeof(uint32_t)
            );

            innerPayload.WriteData(
                &audio.channels,
                sizeof(uint32_t)
            );

            uint32_t size =
                (uint32_t)audio.pcm.size();

            innerPayload.WriteData(
                &size,
                sizeof(uint32_t)
            );

            innerPayload.WriteData(
                audio.pcm.data(),
                size
            );

            innerPayload.Align16();
        }

        break;
    }

    default:
    {
        std::cout
            << "  → No exporter (param-only chunk)\n";

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