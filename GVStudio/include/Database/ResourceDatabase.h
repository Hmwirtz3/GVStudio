#pragma once

#include <string>
#include <vector>
#include <memory>

struct ResourceNode
{
    std::string name;
    std::string fullPath;
    bool isFolder = false;

    ResourceNode* parent = nullptr;
    std::vector<std::unique_ptr<ResourceNode>> children;
};

class ResourceDatabase
{
public:
    ResourceDatabase();

    void Clear();
    void BuildFolderTree(const std::string& rootPath);

    const ResourceNode* GetRoot() const;

private:
    std::unique_ptr<ResourceNode> m_root;

    ResourceNode* FindOrCreateFolder(
        ResourceNode* parent,
        const std::string& name);
};
