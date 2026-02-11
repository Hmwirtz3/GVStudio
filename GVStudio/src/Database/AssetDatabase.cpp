#include "Database/AssetDatabase.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

static std::string ToLower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return s;
}

void AssetDatabase::Clear()
{
    m_assets.clear();
}

void AssetDatabase::Scan(const std::string& resourceRoot)
{
    Clear();

    if (!fs::exists(resourceRoot))
        return;

    for (auto& entry : fs::recursive_directory_iterator(resourceRoot))
    {
        if (!entry.is_regular_file())
            continue;
        ProcessFile(resourceRoot, entry.path().string());
    }
}

void AssetDatabase::ProcessFile(const std::string& root, const std::string& fullPath)
{
    std::string ext = ToLower(fs::path(fullPath).extension().string());

    AssetEntry asset;

    asset.path = fs::relative(fullPath, root).generic_string();
    asset.name = fs::path(fullPath).stem().string();

    if (ext == ".obj")
        asset.type = GV_CHUNK_STATIC_MESH;
    else if (ext == ".png" || ext == ".jpg" || ext == ".bmp")
        asset.type == GV_CHUNK_TEXTURE;
    
    

    

}