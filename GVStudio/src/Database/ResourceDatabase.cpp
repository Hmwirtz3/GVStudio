#include "Database/ResourceDatabase.h"

#include <filesystem>

namespace fs = std::filesystem;

ResourceDatabase::ResourceDatabase()
{
    Clear();
}

void ResourceDatabase::Clear()
{
    m_root = std::make_unique<ResourceNode>();
    m_root->name = "Resources";
    m_root->isFolder = true;
    m_root->parent = nullptr;
    m_root->fullPath.clear();
}

const ResourceNode* ResourceDatabase::GetRoot() const
{
    return m_root.get();
}

void ResourceDatabase::BuildFolderTree(const std::string& rootPath)
{
    Clear();

    if (!fs::exists(rootPath) || !fs::is_directory(rootPath))
        return;

    m_root->fullPath = rootPath;

    for (const auto& entry : fs::recursive_directory_iterator(rootPath))
    {
        fs::path relativePath = fs::relative(entry.path(), rootPath);

        ResourceNode* current = m_root.get();

        for (auto it = relativePath.begin(); it != relativePath.end(); ++it)
        {
            bool isLast = std::next(it) == relativePath.end();
            std::string segment = it->string();

            if (isLast)
            {
                if (entry.is_directory())
                {
                    current = FindOrCreateFolder(current, segment);
                }
                else if (entry.is_regular_file())
                {
                    auto fileNode = std::make_unique<ResourceNode>();
                    fileNode->name = segment;
                    fileNode->isFolder = false;
                    fileNode->parent = current;
                    fileNode->fullPath = entry.path().string();

                    current->children.push_back(std::move(fileNode));
                }
            }
            else
            {
                current = FindOrCreateFolder(current, segment);
            }
        }
    }
}

ResourceNode* ResourceDatabase::FindOrCreateFolder(
    ResourceNode* parent,
    const std::string& name)
{
    for (auto& child : parent->children)
    {
        if (child->isFolder && child->name == name)
            return child.get();
    }

    auto folder = std::make_unique<ResourceNode>();
    folder->name = name;
    folder->isFolder = true;
    folder->parent = parent;

    if (parent->fullPath.empty())
        folder->fullPath = name;
    else
        folder->fullPath = parent->fullPath + "/" + name;

    ResourceNode* ptr = folder.get();
    parent->children.push_back(std::move(folder));

    return ptr;
}
