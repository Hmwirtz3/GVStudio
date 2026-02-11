#pragma once

#include "GVFramework/Chunk/Chunk.h"
#include "GVFramework/LogicUnit/LogicUnit.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

struct AssetEntry
{
    std::string name;
    std::string path;
    GV_ChunkType type;
    std::vector<std::string> dependencies;
};

class AssetDatabase
{
public:
    AssetDatabase();

    void Clear();

    void ProcessLogicUnitInstance(const GV_Logic_Unit_Instance& instance);

    const AssetEntry* GetAsset(const std::string& path) const;

    std::vector<const AssetEntry*> GetAssetsByChunk(GV_ChunkType type) const;

private:
    using ExtractorFn = std::function<void(
        const GV_Logic_Unit_Instance&,
        std::vector<std::string>&)>;

    void RegisterExtractors();
    void AddAsset(GV_ChunkType type, const std::string& path);
    void ParseDependencies(AssetEntry& entry);

    int FindParamIndex(const GV_Logic_Unit* def,
        const std::vector<std::string>& names) const;

private:
    std::unordered_map<std::string, AssetEntry> m_assets;
    std::unordered_map<GV_ChunkType, ExtractorFn> m_extractors;
};
