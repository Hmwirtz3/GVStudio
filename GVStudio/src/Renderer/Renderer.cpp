#include "Renderer/Renderer.h"

#include "3rdParty/glad/glad.h"

#include <vector>
#include <iostream>
#include <string>

// ---------------------------
// Small GL helpers
// ---------------------------
static void PrintShaderLog(GLuint shader, const char* label)
{
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len <= 1) return;

    std::string log;
    log.resize((size_t)len);
    glGetShaderInfoLog(shader, len, nullptr, log.data());

    std::cout << "[Renderer] Shader log (" << label << "):\n" << log << "\n";
}

static void PrintProgramLog(GLuint program)
{
    GLint len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    if (len <= 1) return;

    std::string log;
    log.resize((size_t)len);
    glGetProgramInfoLog(program, len, nullptr, log.data());

    std::cout << "[Renderer] Program log:\n" << log << "\n";
}

static GLuint CompileShader(GLenum type, const char* src, const char* label)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);

    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        std::cout << "[Renderer] FAILED compiling shader: " << label << "\n";
        PrintShaderLog(sh, label);
        glDeleteShader(sh);
        return 0;
    }

    // Still print warnings if any
    PrintShaderLog(sh, label);
    return sh;
}

static GLuint LinkProgram(GLuint vs, GLuint fs)
{
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        std::cout << "[Renderer] FAILED linking program\n";
        PrintProgramLog(prog);
        glDeleteProgram(prog);
        return 0;
    }

    // Still print warnings if any
    PrintProgramLog(prog);

    glDetachShader(prog, vs);
    glDetachShader(prog, fs);

    return prog;
}

// ---------------------------
// Renderer
// ---------------------------
void Renderer::Init()
{
    CreateFramebuffer();
    CreateBasicShader();
    CreateGrid();
    CreateCube();
}

void Renderer::Shutdown()
{
    if (m_cubeVBO) { glDeleteBuffers(1, &m_cubeVBO); m_cubeVBO = 0; }
    if (m_cubeVAO) { glDeleteVertexArrays(1, &m_cubeVAO); m_cubeVAO = 0; }

    if (m_gridVBO) { glDeleteBuffers(1, &m_gridVBO); m_gridVBO = 0; }
    if (m_gridVAO) { glDeleteVertexArrays(1, &m_gridVAO); m_gridVAO = 0; }

    if (m_shader) { glDeleteProgram(m_shader); m_shader = 0; }

    if (m_depthRbo) { glDeleteRenderbuffers(1, &m_depthRbo); m_depthRbo = 0; }
    if (m_colorTex) { glDeleteTextures(1, &m_colorTex); m_colorTex = 0; }
    if (m_fbo) { glDeleteFramebuffers(1, &m_fbo); m_fbo = 0; }

    // reset cached state
    m_width = 1;
    m_height = 1;
}

void Renderer::Resize(int width, int height)
{
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;

    if (width == m_width && height == m_height)
        return;

    m_width = width;
    m_height = height;

    // Resize color texture + depth/stencil RBO
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Renderer::Begin(const Mat4& view, const Mat4& proj)
{
    m_view = view;
    m_proj = proj;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glDisable(GL_BLEND);

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::End()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Renderer::GetColorTexture() const
{
    return m_colorTex;
}

// ---------------------------
// Internals
// ---------------------------
void Renderer::CreateFramebuffer()
{
    // Create objects if not yet created
    if (!m_fbo) glGenFramebuffers(1, &m_fbo);
    if (!m_colorTex) glGenTextures(1, &m_colorTex);
    if (!m_depthRbo) glGenRenderbuffers(1, &m_depthRbo);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "[Renderer] FBO incomplete! status = 0x" << std::hex << status << std::dec << "\n";
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::CreateBasicShader()
{
    // Minimal solid-color shader
    const char* vsSrc =
        "#version 330 core\n"
        "layout(location = 0) in vec3 aPos;\n"
        "uniform mat4 uView;\n"
        "uniform mat4 uProj;\n"
        "uniform mat4 uModel;\n"
        "void main(){\n"
        "  gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);\n"
        "}\n";

    const char* fsSrc =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec3 uColor;\n"
        "void main(){\n"
        "  FragColor = vec4(uColor, 1.0);\n"
        "}\n";

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSrc, "BasicVS");
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc, "BasicFS");
    if (!vs || !fs)
        return;

    m_shader = LinkProgram(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!m_shader)
        return;

    // Cache uniform locations (requires header additions; see below)
    m_uViewLoc = glGetUniformLocation(m_shader, "uView");
    m_uProjLoc = glGetUniformLocation(m_shader, "uProj");
    m_uModelLoc = glGetUniformLocation(m_shader, "uModel");
    m_uColorLoc = glGetUniformLocation(m_shader, "uColor");

    if (m_uViewLoc < 0 || m_uProjLoc < 0 || m_uModelLoc < 0 || m_uColorLoc < 0)
    {
        std::cout << "[Renderer] Warning: one or more uniforms not found in basic shader.\n";
    }
}

void Renderer::CreateGrid()
{
    // Simple XZ grid centered at origin.
    // Lines from -N..+N in both axes, spacing 1.
    const int N = 20; // grid extends to +/-N
    std::vector<float> verts;
    verts.reserve((size_t)((N * 2 + 1) * 4) * 3);

    for (int i = -N; i <= N; ++i)
    {
        // Line parallel to X (varying X, fixed Z=i)
        verts.push_back((float)-N); verts.push_back(0.0f); verts.push_back((float)i);
        verts.push_back((float)+N); verts.push_back(0.0f); verts.push_back((float)i);

        // Line parallel to Z (varying Z, fixed X=i)
        verts.push_back((float)i);  verts.push_back(0.0f); verts.push_back((float)-N);
        verts.push_back((float)i);  verts.push_back(0.0f); verts.push_back((float)+N);
    }

    m_gridVertexCount = (int)(verts.size() / 3);

    if (!m_gridVAO) glGenVertexArrays(1, &m_gridVAO);
    if (!m_gridVBO) glGenBuffers(1, &m_gridVBO);

    glBindVertexArray(m_gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(float)), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * (GLsizei)sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::CreateCube()
{
    // Unit cube centered at origin (36 verts, 12 triangles).
    // Only position, no normals/uv.
    const float v[] =
    {
        // +X
        0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
        0.5f,-0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,
        // -X
       -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
       -0.5f,-0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f,
       // +Y
      -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
      -0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
      // -Y
     -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,
     -0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
     // +Z
    -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,
    // -Z
    0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
    0.5f,-0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f
    };

    m_cubeVertexCount = 36;

    if (!m_cubeVAO) glGenVertexArrays(1, &m_cubeVAO);
    if (!m_cubeVBO) glGenBuffers(1, &m_cubeVBO);

    glBindVertexArray(m_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(v), v, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * (GLsizei)sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// ---------------------------
// Draw calls
// ---------------------------
void Renderer::DrawGrid()
{
    if (!m_shader || !m_gridVAO || m_gridVertexCount <= 0)
        return;

    glUseProgram(m_shader);

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);

    Mat4 model = Mat4::Identity();
    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);

    glUniform3f(m_uColorLoc, 0.40f, 0.40f, 0.40f);

    glBindVertexArray(m_gridVAO);
    glDrawArrays(GL_LINES, 0, m_gridVertexCount);
    glBindVertexArray(0);
}

void Renderer::DrawCube(const Mat4& model)
{
    if (!m_shader || !m_cubeVAO || m_cubeVertexCount <= 0)
        return;

    glUseProgram(m_shader);

    glUniformMatrix4fv(m_uViewLoc, 1, GL_FALSE, m_view.m);
    glUniformMatrix4fv(m_uProjLoc, 1, GL_FALSE, m_proj.m);

    glUniformMatrix4fv(m_uModelLoc, 1, GL_FALSE, model.m);

    glUniform3f(m_uColorLoc, 0.85f, 0.25f, 0.20f);

    glBindVertexArray(m_cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_cubeVertexCount);
    glBindVertexArray(0);
}