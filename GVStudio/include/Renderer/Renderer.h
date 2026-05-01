#pragma once

#include "MiniMath/MiniMath.h"
#include "Renderer/MeshBaker.h"
#include "Renderer/MeshTypes.h"
#include "Renderer/MeshSystem.h"
#include "Renderer/TextureSystem.h"
#include "Renderer/GraphicsDevice.h"
#include "Renderer/Shaders/Shader.h"
#include "Exporters/ExportContext.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

struct BakedEntry
{
    Mesh mesh;
    Mat4 lastModel;
    bool baked = false;
};

class Renderer
{
public:
    void Init();
    void Shutdown();
    void Resize(int width, int height);

    void Begin(const Mat4& view, const Mat4& proj);
    void End();

    void SetBakedLights(const std::vector<BakedLight>& lights);
    

    void DrawGrid();
    void DrawCube(const Mat4& model);
    void DrawCameraGizmo(const Vec3& pos, const Vec3& rot);
    void DrawModel(const std::string& path, const Mat4& model);

    const Mesh* GetBakedMesh(const std::string& key) const;

    GV::MeshSystem& GetMeshSystem();

    uint32_t GetColorTexture() const;

    void ClearScene();

    GV_ExportContext& GetExportContext();

private:
    void CreateFramebuffer();
    void CreateBasicShader();
    void CreateGrid();
    void CreateCube();


    void UploadMesh(Mesh& mesh);
    void FreeMeshGpu(Mesh& mesh);
    Mesh& GetBakedMeshForDraw(const std::string& key, const Mesh& source, const Mat4& model);

private:
    int m_width = 1280;
    int m_height = 720;

    uint32_t m_fbo = 0;
    uint32_t m_colorTex = 0;
    uint32_t m_depthRbo = 0;

    uint32_t m_gridVAO = 0;
    uint32_t m_gridVBO = 0;
    int m_gridVertexCount = 0;

    uint32_t m_cubeVAO = 0;
    uint32_t m_cubeVBO = 0;
    int m_cubeVertexCount = 0;

    Mat4 m_view;
    Mat4 m_proj;

    GV::Shader m_shader;
    int m_uViewLoc = -1;
    int m_uProjLoc = -1;
    int m_uModelLoc = -1;
    int m_uColorLoc = -1;
    int m_uUseTextureLoc = -1;
    int m_uTexLoc = -1;

    GV_ExportContext m_exportContext;

    GV::GraphicsDevice m_graphics;
    GV::MeshSystem m_meshSystem;
    GV::TextureSystem m_textureSystem;

    std::vector<BakedLight> m_bakedLights;

    std::unordered_map<std::string, Mesh> m_bakedMeshes;
    std::unordered_map<std::string, int> m_instanceDrawIndex;
};