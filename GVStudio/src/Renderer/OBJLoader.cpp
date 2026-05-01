#include "Renderer/OBJLoader.h"
#include "Renderer/MeshTypes.h"
#include "MiniMath/MiniMath.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <cstdio>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <fstream>

namespace fs = std::filesystem;

namespace GV
{
    static std::unordered_map<std::string, std::string>
        ParseMTLTextures(const std::string& path)
    {
        std::unordered_map<std::string, std::string> materials;

        std::ifstream file(path);
        std::string line;

        std::string currentMaterial;

        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string type;
            ss >> type;

            if (type == "newmtl")
            {
                ss >> currentMaterial;
            }
            else if (type == "map_Kd")
            {
                std::string tex;
                std::getline(ss, tex);

                tex.erase(0, tex.find_first_not_of(" \t\r\n"));
                tex.erase(tex.find_last_not_of(" \t\r\n") + 1);

                fs::path texPath(tex);

                if (!texPath.is_absolute())
                    texPath = fs::path(path).parent_path() / texPath;

                std::string fullPath = texPath.string();
                std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                materials[currentMaterial] = fullPath;

                printf("[MTL] Material '%s' -> Texture '%s'\n",
                    currentMaterial.c_str(),
                    fullPath.c_str());
            }
        }

        return materials;
    }

    Mesh LoadOBJ(const std::string& path)
    {
        Mesh mesh;

        FILE* f = nullptr;
        fopen_s(&f, path.c_str(), "r");

        if (!f)
            return mesh;

        std::vector<Vec3> positions;
        std::vector<Vec2> uvs;

        std::unordered_map<std::string, std::string> materials;

        std::string mtlFile;
        std::string currentMaterial;

        struct BuildMesh
        {
            std::vector<float> vertices;
        };

        std::unordered_map<std::string, BuildMesh> builders;
        std::vector<std::string> materialOrder;

        BuildMesh* currentBuilder = nullptr;

        char line[1024];

        while (fgets(line, sizeof(line), f))
        {
            char* ptr = line;

            if (ptr[0] == 'v' && ptr[1] == ' ')
            {
                Vec3 v;
                sscanf_s(ptr, "v %f %f %f", &v.x, &v.y, &v.z);
                positions.push_back(v);
            }
            else if (ptr[0] == 'v' && ptr[1] == 't')
            {
                Vec2 uv;
                sscanf_s(ptr, "vt %f %f", &uv.x, &uv.y);
                uvs.push_back(uv);
            }
            else if (strncmp(ptr, "mtllib", 6) == 0)
            {
                char name[256];
                sscanf_s(ptr, "mtllib %s", name, (unsigned)_countof(name));
                mtlFile = name;

                printf("[OBJ] mtllib '%s'\n", mtlFile.c_str());
            }
            else if (strncmp(ptr, "usemtl", 6) == 0)
            {
                char name[256];
                sscanf_s(ptr, "usemtl %s", name, (unsigned)_countof(name));

                currentMaterial = name;

                if (builders.find(currentMaterial) == builders.end())
                {
                    builders[currentMaterial] = BuildMesh{};
                    materialOrder.push_back(currentMaterial);
                }

                currentBuilder = &builders[currentMaterial];

                if (currentBuilder->vertices.empty())
                    currentBuilder->vertices.reserve(800000);

                printf("[OBJ] usemtl '%s'\n", currentMaterial.c_str());
            }
            else if (ptr[0] == 'f' && ptr[1] == ' ')
            {
                int v[4], t[4], n[4];

                int count = sscanf_s(
                    ptr,
                    "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                    &v[0], &t[0], &n[0],
                    &v[1], &t[1], &n[1],
                    &v[2], &t[2], &n[2],
                    &v[3], &t[3], &n[3]
                );

                int verts = count / 3;

                if (!currentBuilder)
                {
                    currentMaterial = "default";

                    if (builders.find(currentMaterial) == builders.end())
                    {
                        builders[currentMaterial] = BuildMesh{};
                        materialOrder.push_back(currentMaterial);
                    }

                    currentBuilder = &builders[currentMaterial];
                }

                for (int i = 1; i < verts - 1; i++)
                {
                    int tri[3] = { 0, i, i + 1 };

                    for (int k = 0; k < 3; k++)
                    {
                        int pi = v[tri[k]] - 1;
                        int ti = t[tri[k]] - 1;

                        const Vec3& p = positions[pi];

                        Vec2 uv(0, 0);
                        if (ti >= 0 && ti < (int)uvs.size())
                        {
                            uv = uvs[ti];
                            uv.y = 1.0f - uv.y;
                        }

                        currentBuilder->vertices.push_back(p.x);
                        currentBuilder->vertices.push_back(p.y);
                        currentBuilder->vertices.push_back(p.z);
                        currentBuilder->vertices.push_back(uv.x);
                        currentBuilder->vertices.push_back(uv.y);
                        currentBuilder->vertices.push_back(1.0f);
                        currentBuilder->vertices.push_back(1.0f);
                        currentBuilder->vertices.push_back(1.0f);
                    }
                }
            }
        }

        fclose(f);

        if (!mtlFile.empty())
        {
            fs::path objPath(path);
            fs::path mtlPath = objPath.parent_path() / mtlFile;

            printf("[OBJ] Loading MTL '%s'\n", mtlPath.string().c_str());

            materials = ParseMTLTextures(mtlPath.string());
        }

        for (const std::string& matName : materialOrder)
        {
            auto builderIt = builders.find(matName);
            if (builderIt == builders.end())
                continue;

            auto& data = builderIt->second;

            if (data.vertices.empty())
                continue;

            SubMesh part;

            part.vertexCount = (int)(data.vertices.size() / 8);
            part.vertices = data.vertices;

            printf("[OBJ] Build SubMesh Material '%s'\n", matName.c_str());
            printf("[OBJ] Vertex Count %d\n", part.vertexCount);

            auto it = materials.find(matName);
            if (it != materials.end())
            {
                part.texturePath = it->second;

                printf("[OBJ] Texture '%s'\n", part.texturePath.c_str());

                fs::path diffusePath = part.texturePath;
                std::string baseName = diffusePath.stem().string() + "_n";
                fs::path folder = diffusePath.parent_path();

                fs::path candidates[] =
                {
                    folder / (baseName + diffusePath.extension().string()),
                    folder / (baseName + ".png"),
                    folder / (baseName + ".bmp"),
                    folder / (baseName + ".tga"),
                    folder / (baseName + ".jpg")
                };

                bool found = false;

                for (const auto& p : candidates)
                {
                    printf("[DEBUG] Checking Normal Candidate: '%s'\n", p.string().c_str());

                    if (fs::exists(p))
                    {
                        std::string normalFull = p.string();
                        std::replace(normalFull.begin(), normalFull.end(), '\\', '/');

                        part.normalPath = normalFull;

                        printf("[OBJ] Normal Map '%s'\n", normalFull.c_str());

                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    printf("[OBJ] No normal map for '%s'\n", diffusePath.string().c_str());
                }
            }
            else
            {
                printf("[OBJ] MISSING TEXTURE FOR MATERIAL '%s'\n", matName.c_str());
            }

            mesh.parts.push_back(part);
        }

        return mesh;
    }
}