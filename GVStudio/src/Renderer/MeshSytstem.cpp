#include "Renderer/MeshSystem.h"
#include "Renderer/OBJLoader.h"

namespace GV
{
    void MeshSystem::Clear()
    {
        m_sourceMeshes.clear();
    }

    Mesh& MeshSystem::GetOrLoad(const std::string& path)
    {
        auto it = m_sourceMeshes.find(path);
        if (it != m_sourceMeshes.end())
            return it->second;

        Mesh mesh = LoadOBJ(path);
        m_sourceMeshes[path] = mesh;
        return m_sourceMeshes[path];
    }

    const Mesh* MeshSystem::Get(const std::string& path) const
    {
        auto it = m_sourceMeshes.find(path);
        if (it != m_sourceMeshes.end())
            return &it->second;

        return nullptr;
    }
}