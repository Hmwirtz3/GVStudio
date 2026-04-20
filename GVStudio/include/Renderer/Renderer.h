#pragma once

#include "MiniMath/MiniMath.h"
#include "Exporters/ExportContext.h"
#include <string>
#include <unordered_map>
#include <vector>



struct BakedLight
{
    int type = 0; // 0 = point, 1 = directional

    Vec3 position;
    Vec3 direction;

    Vec3 color = { 1.0f, 1.0f, 1.0f };

    float intensity = 1.0f;
    float range = 10.0f;
    float falloff = 2.0f;
};



struct Triangle
{
    Vec3 a, b, c;
};



struct SubMesh
{
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int texture = 0;
    int vertexCount = 0;

    std::string texturePath;

    
    std::vector<float> vertices;
};



struct Mesh
{
    std::vector<SubMesh> parts;
};

struct SceneBakeEntry
{
    Mesh* mesh;
    Mat4 model;
};
struct MeshInstance
{
    Mesh mesh;
    Mat4 model;
};





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
    void DrawModel(const std::string& path, const Mat4& model);

    void DrawCameraGizmo(const Vec3& pos, const Vec3& rot);

    void BakeScene();

    
    void SetBakedLights(const std::vector<BakedLight>& lights);
    void BakeMesh(Mesh& mesh, const Mat4& model);
    void RebakeModel(const std::string& path);
    GV_MeshData ConvertToExportMesh(const Mesh& mesh);
    const std::unordered_map<std::string, Mesh>& GetMeshCache() const;
    std::unordered_map<std::string, int> m_instanceDrawIndex;

    struct BakedInstance
    {
        std::vector<SubMesh> parts;
    };

    std::unordered_map<std::string, std::vector<BakedInstance>> m_bakedInstances;
    std::unordered_map<std::string, Mesh> m_meshCacheOriginal;

    unsigned int GetColorTexture() const;
    std::unordered_map<std::string, std::vector<Mat4>> m_meshInstances;
    std::unordered_map<std::string, Mesh> m_sourceMeshes;

private:
    void CreateFramebuffer();
    void CreateBasicShader();
    void CreateGrid();
    void CreateCube();

    Mesh LoadOBJ(const std::string& path);

private:
    unsigned int m_fbo = 0;
    unsigned int m_colorTex = 0;
    unsigned int m_depthRbo = 0;

    unsigned int m_shader = 0;

    unsigned int m_gridVAO = 0;
    unsigned int m_gridVBO = 0;

    unsigned int m_cubeVAO = 0;
    unsigned int m_cubeVBO = 0;


    std::unordered_map<std::string, Mesh> m_meshCache;

  
    std::vector<BakedLight> m_bakedLights;

    int m_width = 1;
    int m_height = 1;

    int m_uViewLoc = -1;
    int m_uProjLoc = -1;
    int m_uModelLoc = -1;
    int m_uColorLoc = -1;

    int m_gridVertexCount = 0;
    int m_cubeVertexCount = 0;

    Mat4 m_view;
    Mat4 m_proj;
};