#include "Renderer/Renderer.h"
#include "Renderer/MeshBaker.h"
#include "Renderer/MeshExport.h"

#include "3rdParty/glad/glad.h"

#include <cmath>
#include <vector>

void Renderer::Init()
{
    m_graphics.Init();

    CreateFramebuffer();
    CreateBasicShader();
    CreateGrid();
    CreateCube();
}

void Renderer::Shutdown()
{
    for (auto& pair : m_bakedMeshes)
        FreeMeshGpu(pair.second);

    m_bakedMeshes.clear();

    if (m_gridVBO)
        glDeleteBuffers(1, &m_gridVBO);

    if (m_gridVAO)
        glDeleteVertexArrays(1, &m_gridVAO);

    if (m_cubeVBO)
        glDeleteBuffers(1, &m_cubeVBO);

    if (m_cubeVAO)
        glDeleteVertexArrays(1, &m_cubeVAO);

    if (m_depthRbo)
        glDeleteRenderbuffers(1, &m_depthRbo);

    if (m_colorTex)
        glDeleteTextures(1, &m_colorTex);

    if (m_fbo)
        glDeleteFramebuffers(1, &m_fbo);

    m_gridVBO = 0;
    m_gridVAO = 0;
    m_cubeVBO = 0;
    m_cubeVAO = 0;
    m_depthRbo = 0;
    m_colorTex = 0;
    m_fbo = 0;

    m_textureSystem.Clear();
    m_meshSystem.Clear();
}

void Renderer::Resize(int width, int height)
{
    m_width = width;
    m_height = height;
    CreateFramebuffer();
}

void Renderer::Begin(const Mat4& view, const Mat4& proj)
{
    m_view = view;
    m_proj = proj;

    m_instanceDrawIndex.clear();

    m_graphics.BeginFrame(m_fbo, m_width, m_height);
}

void Renderer::End()
{
    m_graphics.EndFrame();
}

void Renderer::SetBakedLights(const std::vector<BakedLight>& lights)
{
    m_bakedLights = lights;
}

void Renderer::ClearScene()
{
    for (auto& pair : m_bakedMeshes)
        FreeMeshGpu(pair.second);

    m_bakedMeshes.clear();
    m_instanceDrawIndex.clear();

    m_exportContext.Clear();

    m_meshSystem.Clear();
    m_textureSystem.Clear();
}

GV_ExportContext& Renderer::GetExportContext()
{
    return m_exportContext;
}

void Renderer::CreateFramebuffer()
{
    if (m_fbo)
    {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteTextures(1, &m_colorTex);
        glDeleteRenderbuffers(1, &m_depthRbo);

        m_fbo = 0;
        m_colorTex = 0;
        m_depthRbo = 0;
    }

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        m_width,
        m_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_colorTex,
        0
    );

    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH24_STENCIL8,
        m_width,
        m_height
    );

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER,
        m_depthRbo
    );

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::CreateBasicShader()
{
    const char* vs = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aColor;

uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uModel;

out vec2 vUV;
out vec3 vColor;

void main()
{
    vUV = aUV;
    vColor = aColor;
    gl_Position = uProj * uView * uModel * vec4(aPos,1.0);
})";

    const char* fs = R"(#version 330 core
in vec2 vUV;
in vec3 vColor;

out vec4 FragColor;

uniform sampler2D uTex;
uniform vec3 uColor;
uniform int uUseTexture;

void main()
{
    vec3 baseColor;

    if(uUseTexture == 1)
        baseColor = texture(uTex, vUV).rgb;
    else
        baseColor = uColor;

    FragColor = vec4(baseColor * vColor, 1.0);
})";

    m_shader.Create(vs, fs);

    m_uViewLoc = m_shader.GetUniform("uView");
    m_uProjLoc = m_shader.GetUniform("uProj");
    m_uModelLoc = m_shader.GetUniform("uModel");
    m_uColorLoc = m_shader.GetUniform("uColor");
    m_uUseTextureLoc = m_shader.GetUniform("uUseTexture");
    m_uTexLoc = m_shader.GetUniform("uTex");
}

void Renderer::CreateGrid()
{
    std::vector<float> vertices;

    int size = 20;

    for (int i = -size; i <= size; ++i)
    {
        vertices.push_back((float)i);
        vertices.push_back(0.0f);
        vertices.push_back((float)-size);

        vertices.push_back((float)i);
        vertices.push_back(0.0f);
        vertices.push_back((float)size);

        vertices.push_back((float)-size);
        vertices.push_back(0.0f);
        vertices.push_back((float)i);

        vertices.push_back((float)size);
        vertices.push_back(0.0f);
        vertices.push_back((float)i);
    }

    m_gridVertexCount = (int)(vertices.size() / 3);

    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);

    glBindVertexArray(m_gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glBindVertexArray(0);
}

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

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glBindVertexArray(0);
}

void Renderer::UploadMesh(Mesh& mesh)
{
    for (SubMesh& part : mesh.parts)
    {
        if (part.vertices.empty() || part.vertexCount <= 0)
            continue;

        if (!part.vao)
            glGenVertexArrays(1, &part.vao);

        if (!part.vbo)
            glGenBuffers(1, &part.vbo);

        glBindVertexArray(part.vao);
        glBindBuffer(GL_ARRAY_BUFFER, part.vbo);

        glBufferData(
            GL_ARRAY_BUFFER,
            part.vertices.size() * sizeof(float),
            part.vertices.data(),
            GL_DYNAMIC_DRAW
        );

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(float),
            (void*)0
        );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(float),
            (void*)(3 * sizeof(float))
        );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(
            2,
            3,
            GL_FLOAT,
            GL_FALSE,
            8 * sizeof(float),
            (void*)(5 * sizeof(float))
        );

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Renderer::FreeMeshGpu(Mesh& mesh)
{
    for (SubMesh& part : mesh.parts)
    {
        if (part.vbo)
            glDeleteBuffers(1, &part.vbo);

        if (part.vao)
            glDeleteVertexArrays(1, &part.vao);

        part.vbo = 0;
        part.vao = 0;
    }
}

Mesh& Renderer::GetBakedMeshForDraw(
    const std::string& key,
    const Mesh& source,
    const Mat4& model)
{
    Mesh& baked = m_bakedMeshes[key];

    if (baked.parts.size() != source.parts.size())
    {
        FreeMeshGpu(baked);
        baked.parts.clear();
        baked.parts.resize(source.parts.size());
    }

    for (size_t i = 0; i < source.parts.size(); ++i)
    {
        const SubMesh& src = source.parts[i];
        SubMesh& dst = baked.parts[i];

        dst.vertexCount = src.vertexCount;
        dst.vertices = src.vertices;
        dst.texturePath = src.texturePath;
    }

    GV::MeshBaker::BakeMesh(baked, model, m_bakedLights);

    GV_MeshData exportMesh = GV::ConvertToExportMesh(baked);
    m_exportContext.SetMeshData(key, exportMesh);

    UploadMesh(baked);

    return baked;
}

GV::MeshSystem& Renderer::GetMeshSystem()
{
    return m_meshSystem;
}

const Mesh* Renderer::GetBakedMesh(const std::string& key) const
{
    auto it = m_bakedMeshes.find(key);

    if (it == m_bakedMeshes.end())
        return nullptr;

    return &it->second;
}

void Renderer::DrawGrid()
{
    m_shader.Bind();

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);

    Mat4 model = Mat4::Identity();
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);

    glUniform3f(m_uColorLoc, 0.3f, 0.3f, 0.3f);
    glUniform1i(m_uUseTextureLoc, 0);

    glDisableVertexAttribArray(2);
    glVertexAttrib3f(2, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(m_gridVAO);
    glDrawArrays(GL_LINES, 0, m_gridVertexCount);
    glBindVertexArray(0);
}

void Renderer::DrawCube(const Mat4& model)
{
    m_shader.Bind();

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);

    glUniform3f(m_uColorLoc, 0.8f, 0.2f, 0.2f);
    glUniform1i(m_uUseTextureLoc, 0);

    glDisableVertexAttribArray(2);
    glVertexAttrib3f(2, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(m_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_cubeVertexCount);
    glBindVertexArray(0);
}

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

    uint32_t vao = 0;
    uint32_t vbo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(verts),
        verts,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        0
    );

    glDisableVertexAttribArray(2);
    glVertexAttrib3f(2, 1.0f, 1.0f, 1.0f);

    m_shader.Bind();

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);

    Mat4 identity = Mat4::Identity();
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, identity.m);

    glUniform3f(m_uColorLoc, 0.0f, 1.0f, 0.0f);
    glUniform1i(m_uUseTextureLoc, 0);

    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, 2);
    glLineWidth(1.0f);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Renderer::DrawModel(const std::string& path, const Mat4& model)
{
    Mesh& source = m_meshSystem.GetOrLoad(path);

    if (source.parts.empty())
        return;

    int& index = m_instanceDrawIndex[path];

    std::string key;

    if (index == 0)
        key = path;
    else
        key = path + "_inst_" + std::to_string(index);

    Mesh& mesh = GetBakedMeshForDraw(key, source, model);

    m_shader.Bind();

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, model.m);
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);

    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);

    glUniform3f(m_uColorLoc, 0.8f, 0.8f, 0.8f);
    glUniform1i(m_uTexLoc, 0);

    for (SubMesh& part : mesh.parts)
    {
        if (!part.texturePath.empty())
        {
            uint32_t tex = m_textureSystem.GetOrLoad(part.texturePath);

            if (tex)
            {
                glUniform1i(m_uUseTextureLoc, 1);
                m_graphics.BindTexture(tex);
            }
            else
            {
                glUniform1i(m_uUseTextureLoc, 0);
            }
        }
        else
        {
            glUniform1i(m_uUseTextureLoc, 0);
        }

        glBindVertexArray(part.vao);
        glDrawArrays(GL_TRIANGLES, 0, part.vertexCount);
    }

    glBindVertexArray(0);

    index++;
}

uint32_t Renderer::GetColorTexture() const
{
    return m_colorTex;
}