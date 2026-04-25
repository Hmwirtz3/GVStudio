#include "Renderer/TextureSystem.h"
#include "3rdParty/STB/stb_image.h"
#include "3rdParty/glad/glad.h"

namespace GV
{
    void TextureSystem::Clear()
    {
        for (auto& t : m_textures)
        {
            glDeleteTextures(1, &t.second);
        }

        m_textures.clear();
    }

    uint32_t TextureSystem::GetOrLoad(const std::string& path)
    {
        auto it = m_textures.find(path);
        if (it != m_textures.end())
            return it->second;

        int w, h, c;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &c, 4);
        if (!data)
            return 0;

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            w,
            h,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        m_textures[path] = tex;
        return tex;
    }
}