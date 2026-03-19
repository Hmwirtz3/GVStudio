#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

// ============================================================
// VERTEX
// ============================================================

struct GV_Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

// ============================================================
// SUBMESH
// ============================================================

struct GV_Submesh
{
    std::vector<GV_Vertex> vertices;
    std::vector<uint16_t> indices;
    uint32_t textureID = 0;
};

// ============================================================
// MESH
// ============================================================

struct GV_MeshData
{
    std::vector<GV_Submesh> submeshes;
};

// ============================================================
// CONTEXT
// ============================================================

class GV_ExportContext
{
public:

    // ========================================================
    // PROJECT SETUP
    // ========================================================

    // Sets both project root and resource folder (relative)
    void SetProjectInfo(
        const std::string& projectRoot,
        const std::string& resourceFolder);

    // ========================================================
    // PATH
    // ========================================================

    std::string ResolvePath(const std::string& path) const;
    std::string GetDirectory(const std::string& path) const;

    // ========================================================
    // TEXTURES
    // ========================================================

    uint32_t RegisterTexture(const std::string& name);
    const std::vector<std::string>& GetTextures() const;

    // ========================================================
    // MESH STORAGE
    // ========================================================

    uint32_t RegisterMesh(const std::string& name);
    const GV_MeshData* GetMeshData(const std::string& name) const;
    const std::vector<std::string>& GetMeshes() const;

    // ========================================================
    // LOADERS
    // ========================================================

    bool LoadOBJ(const std::string& inputPath, GV_MeshData& out);
    bool LoadMTL(
        const std::string& path,
        std::unordered_map<std::string, std::string>& materialToTex);

    // ========================================================
    // UTIL
    // ========================================================

    void ParseFaceVertex(const std::string& token, int& pi, int& ti, int& ni);
    void Clear();

private:

    // ========================================================
    // PROJECT DATA
    // ========================================================

    std::string m_projectRoot;
    std::string m_resourceFolder;

    // ========================================================
    // TEXTURES
    // ========================================================

    std::unordered_map<std::string, uint32_t> textureMap;
    std::vector<std::string> textures;

    // ========================================================
    // MESH STORAGE
    // ========================================================

    std::unordered_map<std::string, uint32_t> meshMap;
    std::unordered_map<std::string, GV_MeshData> meshData;
    std::vector<std::string> meshes;
};