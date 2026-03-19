#include "Exporters/ExportContext.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

// ============================================================
// SETTERS
// ============================================================

void GV_ExportContext::SetProjectInfo(
    const std::string& root,
    const std::string& resourceFolder)
{
    m_projectRoot = root;
    m_resourceFolder = resourceFolder;

    std::cout << "[Context] SetProjectInfo\n";
    std::cout << "  Root: " << m_projectRoot << "\n";
    std::cout << "  ResourceFolder: " << m_resourceFolder << "\n";
}

// ============================================================
// PATH
// ============================================================

std::string GV_ExportContext::ResolvePath(const std::string& path) const
{
    std::cout << "\n[ResolvePath]\n";
    std::cout << "  Input: " << path << "\n";
    std::cout << "  ProjectRoot: " << m_projectRoot << "\n";
    std::cout << "  ResourceFolder: " << m_resourceFolder << "\n";

    if (path.empty())
        return "";

    // Absolute path → return as-is
    if (path.find(':') != std::string::npos ||
        path[0] == '/' || path[0] == '\\')
    {
        std::cout << "  Absolute path, using as-is\n";
        return path;
    }

    // Build: projectRoot + resourceFolder + relative asset path
    std::string full = m_projectRoot;

    if (!full.empty() && full.back() != '/' && full.back() != '\\')
        full += "/";

    if (!m_resourceFolder.empty())
    {
        full += m_resourceFolder;

        if (full.back() != '/' && full.back() != '\\')
            full += "/";
    }

    full += path;

    std::replace(full.begin(), full.end(), '\\', '/');

    std::cout << "  Resolved: " << full << "\n";

    return full;
}

std::string GV_ExportContext::GetDirectory(const std::string& path) const
{
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos)
        return "";
    return path.substr(0, pos + 1);
}

// ============================================================
// TEXTURES
// ============================================================

uint32_t GV_ExportContext::RegisterTexture(const std::string& name)
{
    if (name.empty())
        return 0;

    auto it = textureMap.find(name);
    if (it != textureMap.end())
        return it->second;

    uint32_t id = (uint32_t)textures.size();

    textures.push_back(name);
    textureMap[name] = id;

    std::cout << "[Texture] " << name << " ID=" << id << "\n";

    return id;
}

const std::vector<std::string>& GV_ExportContext::GetTextures() const
{
    return textures;
}

// ============================================================
// MESH STORAGE
// ============================================================

const GV_MeshData* GV_ExportContext::GetMeshData(const std::string& name) const
{
    auto it = meshData.find(name);
    if (it != meshData.end())
        return &it->second;
    return nullptr;
}

const std::vector<std::string>& GV_ExportContext::GetMeshes() const
{
    return meshes;
}

uint32_t GV_ExportContext::RegisterMesh(const std::string& name)
{
    if (name.empty())
        return 0;

    auto it = meshMap.find(name);
    if (it != meshMap.end())
        return it->second;

    uint32_t id = (uint32_t)meshes.size();

    meshes.push_back(name);
    meshMap[name] = id;

    std::cout << "[Mesh] " << name << "\n";

    GV_MeshData data;

    if (!LoadOBJ(name, data))
    {
        std::cout << "[Mesh] FAILED LOAD\n";
        return id;
    }

    meshData[name] = data;

    return id;
}

// ============================================================
// FACE PARSER
// ============================================================

void GV_ExportContext::ParseFaceVertex(
    const std::string& token,
    int& pi,
    int& ti,
    int& ni)
{
    pi = ti = ni = 0;

    std::stringstream ss(token);
    std::string a, b, c;

    std::getline(ss, a, '/');
    std::getline(ss, b, '/');
    std::getline(ss, c, '/');

    if (!a.empty()) pi = std::stoi(a);
    if (!b.empty()) ti = std::stoi(b);
    if (!c.empty()) ni = std::stoi(c);
}

// ============================================================
// LOAD MTL
// ============================================================

bool GV_ExportContext::LoadMTL(
    const std::string& path,
    std::unordered_map<std::string, std::string>& materialToTex)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "[MTL] Failed: " << path << "\n";
        return false;
    }

    std::string line;
    std::string currentMat;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "newmtl")
        {
            ss >> currentMat;
        }
        else if (type == "map_Kd")
        {
            std::string tex;
            ss >> tex;
            materialToTex[currentMat] = tex;
        }
    }

    std::cout << "[MTL] Loaded: " << path << "\n";
    return true;
}

// ============================================================
// LOAD OBJ
// ============================================================

bool GV_ExportContext::LoadOBJ(const std::string& inputPath, GV_MeshData& out)
{
    std::string path = ResolvePath(inputPath);

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "[OBJ] FAILED TO OPEN: " << path << "\n";
        return false;
    }

    std::vector<float> pos;
    std::vector<float> norm;
    std::vector<float> uv;

    std::unordered_map<std::string, std::string> materialToTex;
    std::unordered_map<std::string, GV_Submesh> submeshMap;

    std::string currentMat = "default";
    std::string mtlFile;

    std::string baseDir = GetDirectory(path);

    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v")
        {
            float x, y, z;
            ss >> x >> y >> z;
            pos.insert(pos.end(), { x, y, z });
        }
        else if (type == "vn")
        {
            float x, y, z;
            ss >> x >> y >> z;
            norm.insert(norm.end(), { x, y, z });
        }
        else if (type == "vt")
        {
            float u_, v_;
            ss >> u_ >> v_;
            uv.insert(uv.end(), { u_, v_ });
        }
        else if (type == "mtllib")
        {
            ss >> mtlFile;
        }
        else if (type == "usemtl")
        {
            ss >> currentMat;
        }
        else if (type == "f")
        {
            std::string v0, v1, v2;
            ss >> v0 >> v1 >> v2;

            GV_Submesh& sm = submeshMap[currentMat];

            std::string verts[3] = { v0, v1, v2 };

            for (int i = 0; i < 3; i++)
            {
                int pi, ti, ni;
                ParseFaceVertex(verts[i], pi, ti, ni);

                GV_Vertex v{};

                if (pi > 0)
                {
                    int p = (pi - 1) * 3;
                    v.x = pos[p + 0];
                    v.y = pos[p + 1];
                    v.z = pos[p + 2];
                }

                if (ti > 0 && !uv.empty())
                {
                    int t = (ti - 1) * 2;
                    v.u = uv[t + 0];
                    v.v = 1.0f - uv[t + 1];
                }

                if (ni > 0 && !norm.empty())
                {
                    int n = (ni - 1) * 3;
                    v.nx = norm[n + 0];
                    v.ny = norm[n + 1];
                    v.nz = norm[n + 2];
                }

                sm.vertices.push_back(v);
                sm.indices.push_back((uint16_t)sm.vertices.size() - 1);
            }
        }
    }

    if (!mtlFile.empty())
    {
        LoadMTL(baseDir + mtlFile, materialToTex);
    }

    for (auto& pair : submeshMap)
    {
        GV_Submesh& sm = pair.second;

        std::string tex;

        auto texIt = materialToTex.find(pair.first);
        if (texIt != materialToTex.end())
        {
            tex = baseDir + texIt->second;
        }

        sm.textureID = tex.empty() ? 0 : RegisterTexture(tex);

        out.submeshes.push_back(sm);
    }

    std::cout << "[OBJ] Loaded OK: " << path << "\n";

    return true;
}

// ============================================================
// CLEAR
// ============================================================

void GV_ExportContext::Clear()
{
    textureMap.clear();
    textures.clear();

    meshMap.clear();
    meshes.clear();
    meshData.clear();
}