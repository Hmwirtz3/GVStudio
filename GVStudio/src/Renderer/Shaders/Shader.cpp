#include "Renderer/Shaders/Shader.h"
#include "3rdParty/glad/glad.h"

namespace GV
{
    uint32_t Shader::Compile(uint32_t type, const char* src)
    {
        uint32_t id = glCreateShader(type);
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        return id;
    }

    bool Shader::Create(const char* vs, const char* fs)
    {
        uint32_t program = glCreateProgram();

        uint32_t v = Compile(GL_VERTEX_SHADER, vs);
        uint32_t f = Compile(GL_FRAGMENT_SHADER, fs);

        glAttachShader(program, v);
        glAttachShader(program, f);
        glLinkProgram(program);

        glDeleteShader(v);
        glDeleteShader(f);

        m_program = program;
        return true;
    }

    void Shader::Bind() const
    {
        glUseProgram(m_program);
    }

    uint32_t Shader::GetID() const
    {
        return m_program;
    }

    int Shader::GetUniform(const char* name) const
    {
        return glGetUniformLocation(m_program, name);
    }
}