#pragma once

#include <vector>
#include <string>
#include <cstdint>

struct SubMesh
{
    std::vector<float> vertices;
    int vertexCount = 0;
    std::string texturePath;
    std::string normalPath;

    uint32_t vao = 0;
    uint32_t vbo = 0;
};

struct Mesh
{
    std::vector<SubMesh> parts;
};