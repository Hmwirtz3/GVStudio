

#pragma once

#include <vector>
#include <string>
#include <cstdint>

struct ImportedVertex
{
    float position[3];
    float normal[3];
    float uv[2];
};

struct ImportedMesh
{
    std::string name;

    std::vector<ImportedVertex> vertices;
    std::vector<uint32_t> indices;
};

class MeshImporter
{
public:

    static bool ImportMeshes(struct cgltf_data* data, std::vector<ImportedMesh>& meshes);
};

