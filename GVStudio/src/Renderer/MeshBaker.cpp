#include "Renderer/MeshBaker.h"

#include <cmath>
#include <vector>

namespace GV
{
    void MeshBaker::BakeMesh(
        Mesh& mesh,
        const Mat4& model,
        const std::vector<BakedLight>& bakedLights)
    {
        const float m0 = model.m[0];
        const float m1 = model.m[1];
        const float m2 = model.m[2];
        const float m4 = model.m[4];
        const float m5 = model.m[5];
        const float m6 = model.m[6];
        const float m8 = model.m[8];
        const float m9 = model.m[9];
        const float m10 = model.m[10];
        const float m12 = model.m[12];
        const float m13 = model.m[13];
        const float m14 = model.m[14];

        struct LightFast
        {
            Vec3 pos;
            Vec3 colorScaled;
            float rangeScaled;
            float invRangeScaled;
            float falloff;
            int type;
        };

        std::vector<LightFast> lights;
        lights.reserve(bakedLights.size());

        for (const BakedLight& l : bakedLights)
        {
            LightFast lf;
            lf.pos = l.position;

            lf.colorScaled.x = l.color.x * l.intensity;
            lf.colorScaled.y = l.color.y * l.intensity;
            lf.colorScaled.z = l.color.z * l.intensity;

            lf.rangeScaled = l.range * 1.8f;
            lf.invRangeScaled = (lf.rangeScaled > 0.0f) ? (1.0f / lf.rangeScaled) : 0.0f;

            lf.falloff = l.falloff;
            lf.type = l.type;

            lights.push_back(lf);
        }

        for (SubMesh& part : mesh.parts)
        {
            if (part.vertices.empty())
                continue;

            float* verts = part.vertices.data();

            for (int i = 0; i < part.vertexCount; i++)
            {
                const int base = i * 8;

                const float lx = verts[base + 0];
                const float ly = verts[base + 1];
                const float lz = verts[base + 2];

                const float px = m0 * lx + m4 * ly + m8 * lz + m12;
                const float py = m1 * lx + m5 * ly + m9 * lz + m13;
                const float pz = m2 * lx + m6 * ly + m10 * lz + m14;

                float r = 0.03f;
                float g = 0.03f;
                float b = 0.03f;

                for (const LightFast& light : lights)
                {
                    if (light.type == 0)
                    {
                        const float dx = px - light.pos.x;
                        const float dy = py - light.pos.y;
                        const float dz = pz - light.pos.z;

                        const float distSq = dx * dx + dy * dy + dz * dz;
                        const float rangeSq = light.rangeScaled * light.rangeScaled;

                        if (distSq > rangeSq)
                            continue;

                        const float dist = std::sqrt(distSq);

                        float t = dist * light.invRangeScaled;
                        if (t > 1.0f) t = 1.0f;

                        float atten = std::pow(1.0f - t, light.falloff);

                        r += light.colorScaled.x * atten;
                        g += light.colorScaled.y * atten;
                        b += light.colorScaled.z * atten;
                    }
                    else
                    {
                        r += light.colorScaled.x;
                        g += light.colorScaled.y;
                        b += light.colorScaled.z;
                    }
                }

                verts[base + 5] = (r > 1.0f) ? 1.0f : r;
                verts[base + 6] = (g > 1.0f) ? 1.0f : g;
                verts[base + 7] = (b > 1.0f) ? 1.0f : b;
            }
        }
    }
}