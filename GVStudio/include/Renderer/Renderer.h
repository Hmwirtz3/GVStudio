#pragma once

#include "MiniMath/MiniMath.h"

class Renderer
{
public:
    void Init();
    void Shutdown();

    void Resize(int width, int height);

    void Begin(const Mat4& view, const Mat4& proj);
    void End();

    void DrawGrid();
    void DrawCube(const Mat4& model);

    unsigned int GetColorTexture() const;

private:
    void CreateFramebuffer();
    void CreateBasicShader();
    void CreateGrid();
    void CreateCube();

private:
    unsigned int m_fbo = 0;
    unsigned int m_colorTex = 0;
    unsigned int m_depthRbo = 0;

    unsigned int m_shader = 0;

    unsigned int m_gridVAO = 0;
    unsigned int m_gridVBO = 0;

    unsigned int m_cubeVAO = 0;
    unsigned int m_cubeVBO = 0;

    int m_width = 1;
    int m_height = 1;

    // Cached uniform locations
    int m_uViewLoc = -1;
    int m_uProjLoc = -1;
    int m_uModelLoc = -1;
    int m_uColorLoc = -1;

    // Cached vertex counts
    int m_gridVertexCount = 0;
    int m_cubeVertexCount = 0;


    Mat4 m_view;
    Mat4 m_proj;
};
