#include "Renderer/MeshExport.h"
#include <cstdio>

namespace GV
{
    GV_MeshData ConvertToExportMesh(const Mesh& mesh)
    {
        GV_MeshData out;

        

        for (size_t p = 0; p < mesh.parts.size(); p++)
        {
            const SubMesh& part = mesh.parts[p];

            

            GV_Submesh sm;

            for (int i = 0; i < part.vertexCount; i++)
            {
                int base = i * 8;

                GV_Vertex v;

                v.x = part.vertices[base + 0];
                v.y = part.vertices[base + 1];
                v.z = part.vertices[base + 2];

                v.u = part.vertices[base + 3];
                v.v = part.vertices[base + 4];

                v.r = part.vertices[base + 5];
                v.g = part.vertices[base + 6];
                v.b = part.vertices[base + 7];

                
                

                sm.vertices.push_back(v);
                sm.indices.push_back((uint16_t)sm.vertices.size() - 1);
            }

            

            sm.texturePath = part.texturePath;
            out.submeshes.push_back(sm);
        }

       

        return out;
    }
}