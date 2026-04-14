#include "Renderer/Renderer.h"
#include "3rdParty/glad/glad.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

/*===========================================================
UV STRUCT (needed for OBJ vt)
===========================================================*/

struct Vec2
{
    float x;
    float y;

    Vec2() : x(0), y(0) {}
    Vec2(float _x, float _y) : x(_x), y(_y) {}
};

void Renderer::DrawCameraGizmo(const Vec3& pos, const Vec3& rot)
{
    Mat4 model =
        Translate(pos) *
        RotateY(rot.y) *
        RotateX(rot.x) *
        Scale({ 2.0f, 2.0f, 2.0f });

    DrawCube(model);

    Vec3 forward =
    {
        -sinf(rot.y) * cosf(rot.x),
        -sinf(rot.x),
        -cosf(rot.y) * cosf(rot.x)
    };

    Vec3 start = pos;
    Vec3 end =
    {
        pos.x + forward.x * 8.0f,
        pos.y + forward.y * 8.0f,
        pos.z + forward.z * 8.0f
    };

    float verts[] =
    {
        start.x, start.y, start.z,
        end.x,   end.y,   end.z
    };

    unsigned int vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glUseProgram(m_shader);

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);

    Mat4 identity = Mat4::Identity();
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, identity.m);

    glUniform3f(m_uColorLoc, 0.0f, 1.0f, 0.0f);

    int useTexLoc = glGetUniformLocation(m_shader, "uUseTexture");
    glUniform1i(useTexLoc, 0);

    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, 2);
    glLineWidth(1.0f);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}


static unsigned int LoadBMPTexture(const std::string& path)
{
    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "rb");

    if (!f)
    {
        std::cout << "[BMP] Failed to open: " << path << "\n";
        return 0;
    }

    unsigned char header[54];

    if (fread(header, 1, 54, f) != 54)
    {
        std::cout << "[BMP] Failed to read header: " << path << "\n";
        fclose(f);
        return 0;
    }

    // ========================================================
    // CRITICAL: Validate BMP signature
    // ========================================================
    if (header[0] != 'B' || header[1] != 'M')
    {
        std::cout << "[BMP] Not a BMP file: " << path << "\n";
        fclose(f);
        return 0;
    }

    int dataOffset = *(int*)&header[10];
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int bpp = *(short*)&header[28];

    // ========================================================
    // CRITICAL: Sanity checks
    // ========================================================
    if (width <= 0 || height <= 0)
    {
        std::cout << "[BMP] Invalid dimensions: " << path << "\n";
        fclose(f);
        return 0;
    }

    if (bpp != 4 && bpp != 8 && bpp != 24 && bpp != 32)
    {
        std::cout << "[BMP] Unsupported bpp (" << bpp << "): " << path << "\n";
        fclose(f);
        return 0;
    }

    std::cout << "[BMP] w=" << width << " h=" << height << " bpp=" << bpp << "\n";

    std::vector<unsigned char> rgbData;

    // ========================================================
    // INDEXED (4/8 bit)
    // ========================================================
    if (bpp == 8 || bpp == 4)
    {
        int paletteEntries = (bpp == 8) ? 256 : 16;

        std::vector<unsigned char> palette(paletteEntries * 4);
        if (fread(palette.data(), 1, paletteEntries * 4, f) != palette.size())
        {
            std::cout << "[BMP] Failed to read palette: " << path << "\n";
            fclose(f);
            return 0;
        }

        int rowSize = ((width * bpp + 31) / 32) * 4;

        if (rowSize <= 0)
        {
            std::cout << "[BMP] Invalid row size: " << path << "\n";
            fclose(f);
            return 0;
        }

        std::vector<unsigned char> indexData(rowSize * height);

        fseek(f, dataOffset, SEEK_SET);

        if (fread(indexData.data(), 1, indexData.size(), f) != indexData.size())
        {
            std::cout << "[BMP] Failed to read pixel data: " << path << "\n";
            fclose(f);
            return 0;
        }

        rgbData.resize(width * height * 3);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int idx = 0;

                if (bpp == 8)
                {
                    idx = indexData[y * rowSize + x];
                }
                else // 4-bit
                {
                    unsigned char byte = indexData[y * rowSize + (x / 2)];
                    idx = (x % 2 == 0) ? (byte >> 4) : (byte & 0x0F);
                }

                if (idx < 0 || idx >= paletteEntries)
                    idx = 0;

                unsigned char b = palette[idx * 4 + 0];
                unsigned char g = palette[idx * 4 + 1];
                unsigned char r = palette[idx * 4 + 2];

                int dst = (y * width + x) * 3;

                rgbData[dst + 0] = r;
                rgbData[dst + 1] = g;
                rgbData[dst + 2] = b;
            }
        }
    }
    else
    {
        // ====================================================
        // 24 / 32 bit
        // ====================================================
        int bytesPerPixel = bpp / 8;
        int rowSize = ((width * bytesPerPixel + 3) / 4) * 4;

        if (rowSize <= 0)
        {
            std::cout << "[BMP] Invalid row size: " << path << "\n";
            fclose(f);
            return 0;
        }

        std::vector<unsigned char> raw(rowSize * height);

        fseek(f, dataOffset, SEEK_SET);

        if (fread(raw.data(), 1, raw.size(), f) != raw.size())
        {
            std::cout << "[BMP] Failed to read pixel data: " << path << "\n";
            fclose(f);
            return 0;
        }

        rgbData.resize(width * height * bytesPerPixel);

        for (int y = 0; y < height; y++)
        {
            memcpy(
                &rgbData[y * width * bytesPerPixel],
                &raw[y * rowSize],
                width * bytesPerPixel
            );
        }
    }

    fclose(f);
    
    if (rgbData.empty())
    {
        std::cout << "[BMP] No image data: " << path << "\n";
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum format = GL_RGB;
    if (bpp == 32) format = GL_BGRA;
    else if (bpp == 24) format = GL_BGR;

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        rgbData.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    std::cout << "[BMP] Loaded texture: " << path << "\n";

    return tex;
}

/*===========================================================
SHADER HELPERS
===========================================================*/

static unsigned int CompileShader(unsigned int type, const char* src)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    return id;
}

static unsigned int CreateProgram(const char* vs, const char* fs)
{
    unsigned int program = glCreateProgram();
    unsigned int v = CompileShader(GL_VERTEX_SHADER, vs);
    unsigned int f = CompileShader(GL_FRAGMENT_SHADER, fs);

    glAttachShader(program, v);
    glAttachShader(program, f);
    glLinkProgram(program);

    glDeleteShader(v);
    glDeleteShader(f);

    return program;
}

/*===========================================================
MTL PARSER
===========================================================*/

static std::unordered_map<std::string, unsigned int>
ParseMTLTextures(const std::string& path)
{
    std::unordered_map<std::string, unsigned int> materials;

    std::ifstream file(path);
    std::string line;

    std::string currentMaterial;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "newmtl")
        {
            ss >> currentMaterial;
        }
        else if (type == "map_Kd")
        {
            std::string tex;

            // Read full remainder of line (handles spaces)
            std::getline(ss, tex);

            // FULL trim (leading + trailing whitespace)
            tex.erase(0, tex.find_first_not_of(" \t\r\n"));
            tex.erase(tex.find_last_not_of(" \t\r\n") + 1);

            fs::path texPath(tex);

            // Only prepend MTL directory if path is relative
            if (!texPath.is_absolute())
            {
                texPath = fs::path(path).parent_path() / texPath;
            }

            unsigned int texID = LoadBMPTexture(texPath.string());

            materials[currentMaterial] = texID;
        }
    }

    return materials;
}

/*===========================================================
INIT / SHUTDOWN
===========================================================*/

void Renderer::Init()
{
    CreateFramebuffer();
    CreateBasicShader();
    CreateGrid();
    CreateCube();
}

void Renderer::Shutdown()
{
}

void Renderer::Resize(int width, int height)
{
    m_width = width;
    m_height = height;
    CreateFramebuffer();
}

/*===========================================================
FRAMEBUFFER
===========================================================*/

void Renderer::CreateFramebuffer()
{
    if (m_fbo)
    {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteTextures(1, &m_colorTex);
        glDeleteRenderbuffers(1, &m_depthRbo);
    }

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/*===========================================================
SHADER
===========================================================*/

void Renderer::CreateBasicShader()
{
    const char* vs = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;

out vec2 vUV;

void main()
{
    vUV = aUV;
    gl_Position = uProj * uView * uModel * vec4(aPos,1.0);
})";

    const char* fs = R"(#version 330 core
in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uTex;
uniform vec3 uColor;
uniform int uUseTexture;

void main()
{
    if(uUseTexture == 1)
        FragColor = texture(uTex, vUV);
    else
        FragColor = vec4(uColor,1.0);
})";

    m_shader = CreateProgram(vs, fs);

    m_uViewLoc = glGetUniformLocation(m_shader, "uView");
    m_uProjLoc = glGetUniformLocation(m_shader, "uProj");
    m_uModelLoc = glGetUniformLocation(m_shader, "uModel");
    m_uColorLoc = glGetUniformLocation(m_shader, "uColor");
}

/*===========================================================
GRID (UNCHANGED)
===========================================================*/

void Renderer::CreateGrid()
{
    std::vector<float> vertices;

    int size = 20;
    for (int i = -size; i <= size; ++i)
    {
        vertices.push_back((float)i); vertices.push_back(0); vertices.push_back((float)-size);
        vertices.push_back((float)i); vertices.push_back(0); vertices.push_back((float)size);

        vertices.push_back((float)-size); vertices.push_back(0); vertices.push_back((float)i);
        vertices.push_back((float)size); vertices.push_back(0); vertices.push_back((float)i);
    }

    m_gridVertexCount = vertices.size() / 3;

    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);

    glBindVertexArray(m_gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

/*===========================================================
CUBE (UNCHANGED)
===========================================================*/

void Renderer::CreateCube()
{
    float vertices[] =
    {
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
         0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,

        -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,

        -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,

         0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,

        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
         0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,

        -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f
    };

    m_cubeVertexCount = 36;

    glGenVertexArrays(1, &m_cubeVAO);
    glGenBuffers(1, &m_cubeVBO);

    glBindVertexArray(m_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

/*===========================================================
FRAME CONTROL
===========================================================*/

void Renderer::Begin(const Mat4& view, const Mat4& proj)
{
    m_view = view;
    m_proj = proj;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::End()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/*===========================================================
DRAW GRID
===========================================================*/

void Renderer::DrawGrid()
{
    glUseProgram(m_shader);

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);

    Mat4 model = Mat4::Identity();
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);

    glUniform3f(m_uColorLoc, 0.3f, 0.3f, 0.3f);

    glBindVertexArray(m_gridVAO);
    glDrawArrays(GL_LINES, 0, m_gridVertexCount);
    glBindVertexArray(0);
}

/*===========================================================
DRAW CUBE
===========================================================*/

void Renderer::DrawCube(const Mat4& model)
{
    glUseProgram(m_shader);

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);

    glUniform3f(m_uColorLoc, 0.8f, 0.2f, 0.2f);

    int useTexLoc = glGetUniformLocation(m_shader, "uUseTexture");
    glUniform1i(useTexLoc, 0);

    glBindVertexArray(m_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_cubeVertexCount);
    glBindVertexArray(0);
}

/*===========================================================
OBJ LOADER WITH UV + TEXTURE
===========================================================*/

Mesh Renderer::LoadOBJ(const std::string& path)
{
    std::cout << "[OBJ] Attempting to load: " << path << "\n";

    Mesh mesh;

    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "r");

    if (!f)
    {
        std::cout << "[OBJ] FAILED to open file\n";
        return mesh;
    }

    std::vector<Vec3> positions;
    std::vector<Vec2> uvs;

    positions.reserve(200000);
    uvs.reserve(200000);

    std::unordered_map<std::string, unsigned int> materials;

    std::string mtlFile;
    std::string currentMaterial;

    struct BuildMesh
    {
        std::vector<float> vertices;
    };

    std::unordered_map<std::string, BuildMesh> builders;

    BuildMesh* currentBuilder = nullptr;

    char line[1024];

    while (fgets(line, sizeof(line), f))
    {
        char* ptr = line;

        if (ptr[0] == 'v' && ptr[1] == ' ')
        {
            Vec3 v;
            sscanf_s(ptr, "v %f %f %f", &v.x, &v.y, &v.z);
            positions.push_back(v);
        }

        else if (ptr[0] == 'v' && ptr[1] == 't')
        {
            Vec2 uv;
            sscanf_s(ptr, "vt %f %f", &uv.x, &uv.y);
            uvs.push_back(uv);
        }

        else if (strncmp(ptr, "mtllib", 6) == 0)
        {
            char name[256];
            sscanf_s(ptr, "mtllib %s", name, (unsigned)_countof(name));
            mtlFile = name;
        }

        else if (strncmp(ptr, "usemtl", 6) == 0)
        {
            char name[256];
            sscanf_s(ptr, "usemtl %s", name, (unsigned)_countof(name));

            currentMaterial = name;
            currentBuilder = &builders[currentMaterial];

            if (currentBuilder->vertices.empty())
                currentBuilder->vertices.reserve(500000);
        }

        else if (ptr[0] == 'f' && ptr[1] == ' ')
        {
            int v[4], t[4], n[4];

            int count = sscanf_s(
                ptr,
                "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                &v[0], &t[0], &n[0],
                &v[1], &t[1], &n[1],
                &v[2], &t[2], &n[2],
                &v[3], &t[3], &n[3]
            );

            int verts = count / 3;

            if (!currentBuilder)
                currentBuilder = &builders["default"];

            for (int i = 1; i < verts - 1; i++)
            {
                int tri[3] = { 0, i, i + 1 };

                for (int k = 0; k < 3; k++)
                {
                    int pi = v[tri[k]] - 1;
                    int ti = t[tri[k]] - 1;

                    const Vec3& p = positions[pi];

                    Vec2 uv(0, 0);
                    if (ti >= 0 && ti < (int)uvs.size())
                        uv = uvs[ti];

                    currentBuilder->vertices.push_back(p.x);
                    currentBuilder->vertices.push_back(p.y);
                    currentBuilder->vertices.push_back(p.z);
                    currentBuilder->vertices.push_back(uv.x);
                    currentBuilder->vertices.push_back(uv.y);
                }
            }
        }
    }

    fclose(f);

    if (!mtlFile.empty())
    {
        fs::path objPath(path);
        fs::path mtlPath = objPath.parent_path() / mtlFile;

        materials = ParseMTLTextures(mtlPath.string());
    }

    for (auto& pair : builders)
    {
        const std::string& matName = pair.first;
        auto& data = pair.second;

        if (data.vertices.empty())
            continue;

        SubMesh part;

        part.vertexCount = data.vertices.size() / 5;

        auto it = materials.find(matName);
        part.texture = (it != materials.end()) ? it->second : 0;

        glGenVertexArrays(1, &part.vao);
        glGenBuffers(1, &part.vbo);

        glBindVertexArray(part.vao);
        glBindBuffer(GL_ARRAY_BUFFER, part.vbo);

        glBufferData(
            GL_ARRAY_BUFFER,
            data.vertices.size() * sizeof(float),
            data.vertices.data(),
            GL_STATIC_DRAW
        );

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            5 * sizeof(float), (void*)0
        );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE,
            5 * sizeof(float), (void*)(3 * sizeof(float))
        );

        glBindVertexArray(0);

        mesh.parts.push_back(part);
    }

    std::cout << "[OBJ] Loaded mesh parts: " << mesh.parts.size() << "\n";

    return mesh;
}

/*===========================================================
DRAW MODEL
===========================================================*/

void Renderer::DrawModel(const std::string& path, const Mat4& model)
{
    if (m_meshCache.find(path) == m_meshCache.end())
        m_meshCache[path] = LoadOBJ(path);

    Mesh& mesh = m_meshCache[path];
    if (mesh.parts.empty()) return;

    glUseProgram(m_shader);

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);

    glUniform3f(m_uColorLoc, 0.8f, 0.8f, 0.8f);

    int useTexLoc = glGetUniformLocation(m_shader, "uUseTexture");
    int texLoc = glGetUniformLocation(m_shader, "uTex");

    for (const SubMesh& part : mesh.parts)
    {
        if (part.texture)
        {
            glUniform1i(useTexLoc, 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, part.texture);
            glUniform1i(texLoc, 0);
        }
        else
        {
            glUniform1i(useTexLoc, 0);
        }

        glBindVertexArray(part.vao);
        glDrawArrays(GL_TRIANGLES, 0, part.vertexCount);
    }

    glBindVertexArray(0);
}

unsigned int Renderer::GetColorTexture() const
{
    return m_colorTex;
}