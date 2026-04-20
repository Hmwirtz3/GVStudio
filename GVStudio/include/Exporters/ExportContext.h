#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>



struct GV_Vertex
{
    float x, y, z;
    float u, v;
    float r, g, b; 
};



struct GV_Submesh
{
    std::vector<GV_Vertex> vertices;
    std::vector<uint16_t> indices;
    uint32_t textureID = 0;
    std::string texturePath;
};



struct GV_MeshData
{
    std::vector<GV_Submesh> submeshes;
};




class GV_ExportContext
{
public:

 

    // Sets both project root and resource folder (relative)
    void SetProjectInfo(
        const std::string& projectRoot,
        const std::string& resourceFolder);


    uint32_t GetTextureID(const std::string& name) const;

    void SetMeshData(
        const std::string& name,
        const GV_MeshData& data);
    

    std::string ResolvePath(const std::string& path) const;
    std::string GetDirectory(const std::string& path) const;



    uint32_t RegisterTexture(const std::string& name);
    const std::vector<std::string>& GetTextures() const;

    

    uint32_t RegisterMesh(const std::string& name);
    const GV_MeshData* GetMeshData(const std::string& name) const;
    const std::vector<std::string>& GetMeshes() const;

   

    bool LoadOBJ(const std::string& inputPath, GV_MeshData& out);
    bool LoadMTL(
        const std::string& path,
        std::unordered_map<std::string, std::string>& materialToTex);

  

    void ParseFaceVertex(const std::string& token, int& pi, int& ti, int& ni);
    void Clear();

private:

    

    std::string m_projectRoot;
    std::string m_resourceFolder;

  

    std::unordered_map<std::string, uint32_t> textureMap;
    std::vector<std::string> textures;

   

    std::unordered_map<std::string, uint32_t> meshMap;
    std::unordered_map<std::string, GV_MeshData> meshData;
    std::vector<std::string> meshes;
};