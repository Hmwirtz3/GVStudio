#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include <vector>
#include <cstdint>


struct ImportedAnimation;
struct ImportedVertex;
struct ImportedMesh;
struct cgltf_data;
struct cgltf_node;

struct ImportedNode
{
    std::string name;
    int parentIndex;
    int meshIndex;
    float translation[3];
    float rotation[4];
    float scale[3];
};

struct ImportedScene
{
    std::vector<ImportedNode> nodes;
    std::vector<ImportedMesh> meshes;
    std::vector<ImportedAnimation> animations;
};

class GLTFImporter
{
public:
    GLTFImporter();
    ~GLTFImporter();

    bool Import(const std::string& path);
    const ImportedScene& GetScene() const;

private:
    bool ParseNodes();
    int GetNodeIndex(cgltf_node* node);
    int GetMeshIndex(cgltf_node* node);

private:
    cgltf_data* m_data;
    ImportedScene m_scene;
};