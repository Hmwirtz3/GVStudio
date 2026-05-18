#pragma once

#include <vector>

class TerrainRenderer
{
public:
    static void SetCameraPosition(float x, float z);
    static void Build();
    static void MarkDirty();

    static const std::vector<float>& GetVertices();

private:
    static void RebuildVisibleWindow();
};