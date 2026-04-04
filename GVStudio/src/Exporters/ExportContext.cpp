#define _CRT_SECURE_NO_WARNINGS
#include "Exporters/ExportContext.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <cstdio>

// ============================================================
// PATH HELPERS
// ============================================================

static bool IsAbsolutePath(const std::string& path)
{
    if (path.size() > 2 && path[1] == ':') return true;
    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) return true;
    return false;
}

static bool FileExists(const std::string& path)
{
    FILE* f = fopen(path.c_str(), "rb");
    if (f)
    {
        fclose(f);
        return true;
    }
    return false;
}

static bool IsSupportedTexture(const std::string& path)
{
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return (lower.find(".bmp") != std::string::npos);
}

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
    if (path.empty())
        return "";

    if (IsAbsolutePath(path))
        return path;

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
// TEXTURES (FIXED)
// ============================================================

uint32_t GV_ExportContext::RegisterTexture(const std::string& name)
{
    if (name.empty())
        return 0;

    std::string resolved = ResolvePath(name);

    // 🔥 VALIDATION FIRST
    if (!FileExists(resolved))
    {
        std::cout << "[Texture] SKIP (missing): " << resolved << "\n";
        return 0;
    }

    if (!IsSupportedTexture(resolved))
    {
        std::cout << "[Texture] SKIP (unsupported): " << resolved << "\n";
        return 0;
    }

    // Already registered?
    auto it = textureMap.find(resolved);
    if (it != textureMap.end())
        return it->second;

    // IDs start at 1 (0 = no texture)
    uint32_t id = (uint32_t)textures.size() + 1;

    textures.push_back(resolved);
    textureMap[resolved] = id;

    std::cout << "[Texture] OK: " << resolved << " ID=" << id << "\n";

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
    std::string directory = GetDirectory(path);

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
            std::getline(ss, tex);

            // trim leading space
            if (!tex.empty() && tex[0] == ' ')
                tex.erase(0, 1);

            if (!IsAbsolutePath(tex))
                tex = directory + tex;

            std::replace(tex.begin(), tex.end(), '\\', '/');

            materialToTex[currentMat] = tex;
        }
    }

    return true;
}


// ============================================================
// LOAD OBJ (FIXED)
// ============================================================

bool GV_ExportContext::LoadOBJ(const std::string& inputPath, GV_MeshData& out)
{
    std::string path = ResolvePath(inputPath);

    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "[OBJ] FAILED: " << path << "\n";
        return false;
    }

    std::vector<float> pos, norm, uv;
    std::unordered_map<std::string, std::string> materialToTex;
    std::map<std::string, GV_Submesh> submeshMap;

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

                if (pi <= 0) continue;

                GV_Vertex v;

                int p = (pi - 1) * 3;
                v.x = pos[p + 0];
                v.y = pos[p + 1];
                v.z = pos[p + 2];

                v.u = v.v = 0.0f;

                if (ti > 0 && !uv.empty())
                {
                    int t = (ti - 1) * 2;
                    v.u = uv[t + 0];
                    v.v = 1.0f - uv[t + 1];
                }

                sm.vertices.push_back(v);
                sm.indices.push_back((uint16_t)sm.vertices.size() - 1);
            }
        }
    }

    if (!mtlFile.empty())
        LoadMTL(baseDir + mtlFile, materialToTex);

    // Assign textures SAFELY
    for (auto& pair : submeshMap)
    {
        GV_Submesh& sm = pair.second;

        std::string tex;
        auto texIt = materialToTex.find(pair.first);
        if (texIt != materialToTex.end())
            tex = texIt->second;

        if (tex.empty())
        {
            sm.textureID = 0;
        }
        else
        {
            std::string resolved = ResolvePath(tex);

            // 🔥 FILTER HERE FIRST
            if (resolved.find(".bmp") == std::string::npos)
            {
                sm.textureID = 0;
            }
            else
            {
                sm.textureID = RegisterTexture(tex);
            }
        }
        out.submeshes.push_back(sm);
    }

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