#include "Database/AssetDatabase.h"

#include <filesystem>
#include <fstream>

AssetDatabase::AssetDatabase()
{
    RegisterExtractors();
}

void AssetDatabase::Clear()
{
    m_assets.clear();
}

void AssetDatabase::RegisterExtractors()
{
    // TEXTURE
    m_extractors[GV_CHUNK_TEXTURE] =
        [this](const GV_Logic_Unit_Instance& inst,
            std::vector<std::string>& out)
        {
            std::vector<std::string> names =
            {
                "texture",
                "texturePath",
                "albedo",
                "diffuse",
                "path"
            };

            int idx = FindParamIndex(inst.def, names);
            if (idx >= 0 && idx < (int)inst.values.size())
            {
                const std::string& p = inst.values[idx].sval;
                if (!p.empty())
                    out.push_back(p);
            }
        };

    // MODEL
    m_extractors[GV_CHUNK_STATIC_MESH] =
        [this](const GV_Logic_Unit_Instance& inst,
            std::vector<std::string>& out)
        {
            std::vector<std::string> names =
            {
                "model",
                "mesh",
                "clump",
                "modelPath",
                "path"
            };

            int idx = FindParamIndex(inst.def, names);
            if (idx >= 0 && idx < (int)inst.values.size())
            {
                const std::string& p = inst.values[idx].sval;
                if (!p.empty())
                    out.push_back(p);
            }
        };

    // HEIGHTMAP
    m_extractors[GV_CHUNK_HEIGHTMAP] =
        [this](const GV_Logic_Unit_Instance& inst,
            std::vector<std::string>& out)
        {
            std::vector<std::string> names =
            {
                "heightmap",
                "heightmapTexture",
                "heightmapPath",
                "path"
            };

            int idx = FindParamIndex(inst.def, names);
            if (idx >= 0 && idx < (int)inst.values.size())
            {
                const std::string& p = inst.values[idx].sval;
                if (!p.empty())
                    out.push_back(p);
            }
        };
}

int AssetDatabase::FindParamIndex(
    const GV_Logic_Unit* def,
    const std::vector<std::string>& names) const
{
    if (!def)
        return -1;

    for (const auto& want : names)
    {
        for (int i = 0; i < (int)def->params.size(); i++)
        {
            if (def->params[i].name == want)
                return i;
        }
    }

    return -1;
}

void AssetDatabase::ProcessLogicUnitInstance(
    const GV_Logic_Unit_Instance& instance)
{
    if (!instance.def)
        return;

    GV_ChunkType type = instance.def->chunkType;

    auto it = m_extractors.find(type);
    if (it == m_extractors.end())
        return;

    std::vector<std::string> paths;
    it->second(instance, paths);

    for (const auto& p : paths)
        AddAsset(type, p);
}

void AssetDatabase::AddAsset(
    GV_ChunkType type,
    const std::string& path)
{
    if (path.empty())
        return;

    if (m_assets.find(path) != m_assets.end())
        return;

    AssetEntry entry;
    entry.path = path;
    entry.type = type;

    try
    {
        std::filesystem::path fp(path);
        entry.name = fp.stem().string();
    }
    catch (...)
    {
        entry.name = path;
    }

    ParseDependencies(entry);

    m_assets[path] = std::move(entry);
}

void AssetDatabase::ParseDependencies(AssetEntry& entry)
{
    if (entry.type != GV_CHUNK_STATIC_MESH)
        return;

    std::ifstream file(entry.path);
    if (!file.is_open())
        return;

    std::string line;

    while (std::getline(file, line))
    {
        if (line.rfind("mtllib ", 0) == 0)
        {
            std::string mtlFile = line.substr(7);

            std::filesystem::path modelPath(entry.path);
            std::filesystem::path mtlPath =
                modelPath.parent_path() / mtlFile;

            std::ifstream mtl(mtlPath.string());
            if (!mtl.is_open())
                continue;

            std::string mtlLine;

            while (std::getline(mtl, mtlLine))
            {
                if (mtlLine.rfind("map_Kd ", 0) == 0)
                {
                    std::string texName = mtlLine.substr(7);
                    std::filesystem::path texPath =
                        modelPath.parent_path() / texName;

                    entry.dependencies.push_back(texPath.string());
                }
            }
        }
    }
}

const AssetEntry* AssetDatabase::GetAsset(
    const std::string& path) const
{
    auto it = m_assets.find(path);
    if (it == m_assets.end())
        return nullptr;

    return &it->second;
}

std::vector<const AssetEntry*> AssetDatabase::GetAssetsByChunk(
    GV_ChunkType type) const
{
    std::vector<const AssetEntry*> result;

    for (const auto& [path, entry] : m_assets)
    {
        if (entry.type == type)
            result.push_back(&entry);
    }

    return result;
}
