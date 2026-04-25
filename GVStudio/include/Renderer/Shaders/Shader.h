#pragma once

#include <cstdint>

namespace GV
{
    class Shader
    {
    public:
        bool Create(const char* vs, const char* fs);

        void Bind() const;

        uint32_t GetID() const;

        int GetUniform(const char* name) const;

    private:
        uint32_t m_program = 0;

        uint32_t Compile(uint32_t type, const char* src);
    };
}