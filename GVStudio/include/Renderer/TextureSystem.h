#pragma once

#include <unordered_map>
#include <string>
#include <cstdint>

namespace GV
{
    class TextureSystem
    {
    public:
        void Clear();

        uint32_t GetOrLoad(const std::string& path);

    private:
        std::unordered_map<std::string, uint32_t> m_textures;
    };
}