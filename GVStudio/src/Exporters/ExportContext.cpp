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

bool GV_ExportContext::LoadOBJ(const std::string& name, GV_MeshData& out)
{
    std::cout << "[LoadOBJ] Not implemented (using renderer data instead)\n";
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

uint32_t GV_ExportContext::GetTextureID(const std::string& name) const
{
    if (name.empty())
        return 0;

    std::string resolved = ResolvePath(name);

    auto it = textureMap.find(resolved);
    if (it != textureMap.end())
        return it->second;

    return 0;
}

// ============================================================
// TEXTURE FORMAT SUPPORT (FIXED)
// ============================================================

static bool IsSupportedTexture(const std::string& path)
{
    std::string lower = path;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    return (lower.find(".bmp") != std::string::npos ||
        lower.find(".png") != std::string::npos ||
        lower.find(".jpg") != std::string::npos ||
        lower.find(".jpeg") != std::string::npos ||
        lower.find(".tga") != std::string::npos);
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

// ✅ NEW FUNCTION (ONLY ADDITION)
void GV_ExportContext::SetMeshData(
    const std::string& name,
    const GV_MeshData& data)
{
    if (name.empty())
        return;

    meshData[name] = data;

    // ensure it is registered consistently
    if (meshMap.find(name) == meshMap.end())
    {
        uint32_t id = (uint32_t)meshes.size();
        meshes.push_back(name);
        meshMap[name] = id;
    }

    std::cout << "[Mesh] Injected (renderer): " << name << "\n";
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
// TEXTURES (UNCHANGED)
// ============================================================

uint32_t GV_ExportContext::RegisterTexture(const std::string& name)
{
    if (name.empty())
        return 0;

    std::string resolved = ResolvePath(name);

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

    auto it = textureMap.find(resolved);
    if (it != textureMap.end())
        return it->second;

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
// (rest unchanged)
// ============================================================

void GV_ExportContext::Clear()
{
    textureMap.clear();
    textures.clear();
    meshMap.clear();
    meshes.clear();
    meshData.clear();
}