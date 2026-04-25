#pragma once
#include <vector>

class TerrainRenderer
{
public:
    static const std::vector<float>& GetVertices();
    static void Build();
};