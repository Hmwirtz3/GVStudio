#include "Renderer/Renderer.h"
#include "3rdParty/glad/glad.h"

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

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

void Renderer::CreateBasicShader()
{
    const char* vs = R"(#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;
void main()
{
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
})";

    const char* fs = R"(#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main()
{
    FragColor = vec4(uColor, 1.0);
})";

    m_shader = CreateProgram(vs, fs);

    m_uViewLoc = glGetUniformLocation(m_shader, "uView");
    m_uProjLoc = glGetUniformLocation(m_shader, "uProj");
    m_uModelLoc = glGetUniformLocation(m_shader, "uModel");
    m_uColorLoc = glGetUniformLocation(m_shader, "uColor");
}

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

void Renderer::CreateCube()
{
    float vertices[] = {
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,0.5f,-0.5f,
        0.5f,0.5f,-0.5f,   -0.5f,0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,

        -0.5f,-0.5f,0.5f,   0.5f,-0.5f,0.5f,   0.5f,0.5f,0.5f,
        0.5f,0.5f,0.5f,    -0.5f,0.5f,0.5f,   -0.5f,-0.5f,0.5f
    };

    m_cubeVertexCount = 12;

    glGenVertexArrays(1, &m_cubeVAO);
    glGenBuffers(1, &m_cubeVBO);

    glBindVertexArray(m_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

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

void Renderer::DrawCube(const Mat4& model)
{
    glUseProgram(m_shader);
    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);
    glUniform3f(m_uColorLoc, 0.8f, 0.2f, 0.2f);
    glBindVertexArray(m_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_cubeVertexCount);
    glBindVertexArray(0);
}

Mesh Renderer::LoadOBJ(const std::string& path)
{
    std::cout << "[OBJ] Attempting to load: " << path << "\n";

    std::ifstream file(path);
    Mesh mesh;

    if (!file.is_open())
    {
        std::cout << "[OBJ] FAILED to open file.\n";
        return mesh;
    }

    std::vector<Vec3> positions;
    std::vector<float> vertices;

    std::string line;
    size_t faceCount = 0;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v")
        {
            float x, y, z;
            ss >> x >> y >> z;
            positions.emplace_back(x, y, z);
        }
        else if (type == "f")
        {
            faceCount++;

            std::vector<unsigned int> faceIndices;
            std::string vert;

            while (ss >> vert)
            {
                std::stringstream vs(vert);
                std::string indexStr;

                std::getline(vs, indexStr, '/');

                if (indexStr.empty())
                    continue;

                unsigned int index = std::stoi(indexStr);
                faceIndices.push_back(index - 1);
            }

            // Triangulate polygon
            for (size_t i = 1; i + 1 < faceIndices.size(); ++i)
            {
                Vec3 p0 = positions[faceIndices[0]];
                Vec3 p1 = positions[faceIndices[i]];
                Vec3 p2 = positions[faceIndices[i + 1]];

                vertices.insert(vertices.end(), {
                    p0.x, p0.y, p0.z,
                    p1.x, p1.y, p1.z,
                    p2.x, p2.y, p2.z
                    });
            }
        }
    }

    std::cout << "[OBJ] Positions read: " << positions.size() << "\n";
    std::cout << "[OBJ] Faces read: " << faceCount << "\n";
    std::cout << "[OBJ] Generated triangle verts: " << vertices.size() / 3 << "\n";

    if (vertices.empty())
    {
        std::cout << "[OBJ] No vertices generated. Aborting.\n";
        return mesh;
    }

    if (!positions.empty())
    {
        std::cout << "[OBJ] First position: "
            << positions[0].x << ", "
            << positions[0].y << ", "
            << positions[0].z << "\n";
    }

    mesh.vertexCount = (int)(vertices.size() / 3);

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0);

    glBindVertexArray(0);

    std::cout << "[OBJ] GL mesh created. Vertex count: "
        << mesh.vertexCount << "\n";

    return mesh;
}

void Renderer::DrawModel(const std::string& path, const Mat4& model)
{
    if (m_meshCache.find(path) == m_meshCache.end())
        m_meshCache[path] = LoadOBJ(path);

    Mesh& mesh = m_meshCache[path];
    if (!mesh.vao) return;

    glUseProgram(m_shader);
    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);
    glUniform3f(m_uColorLoc, 0.8f, 0.8f, 0.8f);

    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    glBindVertexArray(0);
}

unsigned int Renderer::GetColorTexture() const
{
    return m_colorTex;
}