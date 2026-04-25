#pragma once

#include <unordered_map>
#include <string>
#include "Renderer/MeshTypes.h"


namespace GV
{
    class MeshSystem
    {
    public:
        void Clear();

        Mesh& GetOrLoad(const std::string& path);

        const Mesh* Get(const std::string& path) const;

    private:
        std::unordered_map<std::string, Mesh> m_sourceMeshes;

    };
}