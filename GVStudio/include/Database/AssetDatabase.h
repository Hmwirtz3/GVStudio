#pragma once

#include "GVFramework/Chunk/Chunk.h"

#include <string>
#include <vector>
#include <unordered_map>



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
	void Scan(const std::string& resourceRoot);
	void Clear();

	const AssetEntry* GetAsset(const std::string path) const;
	const AssetEntry* GetTexture(const std::string path) const;
	const AssetEntry* GetModel(const std::string path) const;

private:
	void ProcessFile(const std::string& root, const std::string& fullPath);
	void ParseModelDependencies(AssetEntry& entry);

private:
	std::unordered_map<std::string, AssetEntry> m_assets;
};
