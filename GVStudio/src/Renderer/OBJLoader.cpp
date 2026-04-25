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
                {
                    texPath = fs::path(path).parent_path() / texPath;
                }

                std::string fullPath = texPath.string();
                std::replace(fullPath.begin(), fullPath.end(), '\\', '/');

                materials[currentMaterial] = fullPath;
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
            }
            else if (strncmp(ptr, "usemtl", 6) == 0)
            {
                char name[256];
                sscanf_s(ptr, "usemtl %s", name, (unsigned)_countof(name));

                currentMaterial = name;
                currentBuilder = &builders[currentMaterial];

                if (currentBuilder->vertices.empty())
                    currentBuilder->vertices.reserve(800000);
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
                    currentBuilder = &builders["default"];

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
            materials = ParseMTLTextures(mtlPath.string());
        }

        for (auto& pair : builders)
        {
            const std::string& matName = pair.first;
            auto& data = pair.second;

            if (data.vertices.empty())
                continue;

            SubMesh part;

            part.vertexCount = (int)(data.vertices.size() / 8);
            part.vertices = data.vertices;

            auto it = materials.find(matName);
            if (it != materials.end())
            {
                part.texturePath = it->second;
            }

            mesh.parts.push_back(part);
        }

        return mesh;
    }
}