#include "Renderer/MeshBaker.h"

#include <cmath>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace GV
{
    struct ShadowTri
    {
        Vec3 a, b, c;
    };

    struct GridKey
    {
        int x, y, z;

        bool operator==(const GridKey& o) const
        {
            return x == o.x && y == o.y && z == o.z;
        }
    };

    struct GridKeyHash
    {
        size_t operator()(const GridKey& k) const
        {
            uint32_t h = 2166136261u;
            h = (h ^ (uint32_t)k.x) * 16777619u;
            h = (h ^ (uint32_t)k.y) * 16777619u;
            h = (h ^ (uint32_t)k.z) * 16777619u;
            return (size_t)h;
        }
    };

    static std::vector<ShadowTri> g_tris;
    static std::unordered_map<GridKey, std::vector<int>, GridKeyHash> g_grid;

    static const float CELL = 6.0f;
    static const float BIAS = 0.01f;
    static const float SHADOW = 0.5f;

    static int Cell(float v)
    {
        return (int)std::floor(v / CELL);
    }

    static Vec3 Transform(const Mat4& m, float x, float y, float z)
    {
        return {
            m.m[0] * x + m.m[4] * y + m.m[8] * z + m.m[12],
            m.m[1] * x + m.m[5] * y + m.m[9] * z + m.m[13],
            m.m[2] * x + m.m[6] * y + m.m[10] * z + m.m[14]
        };
    }

    static bool RayTriDist(const Vec3& o, const Vec3& d, const ShadowTri& t, float& outDist)
    {
        const float EPS = 0.00001f;

        Vec3 e1{ t.b.x - t.a.x, t.b.y - t.a.y, t.b.z - t.a.z };
        Vec3 e2{ t.c.x - t.a.x, t.c.y - t.a.y, t.c.z - t.a.z };

        Vec3 p{
            d.y * e2.z - d.z * e2.y,
            d.z * e2.x - d.x * e2.z,
            d.x * e2.y - d.y * e2.x
        };

        float det = e1.x * p.x + e1.y * p.y + e1.z * p.z;
        if (fabs(det) < EPS) return false;

        float inv = 1.0f / det;

        Vec3 s{ o.x - t.a.x, o.y - t.a.y, o.z - t.a.z };

        float u = (s.x * p.x + s.y * p.y + s.z * p.z) * inv;
        if (u < 0.0f || u > 1.0f) return false;

        Vec3 q{
            s.y * e1.z - s.z * e1.y,
            s.z * e1.x - s.x * e1.z,
            s.x * e1.y - s.y * e1.x
        };

        float v = (d.x * q.x + d.y * q.y + d.z * q.z) * inv;
        if (v < 0.0f || u + v > 1.0f) return false;

        float tHit = (e2.x * q.x + e2.y * q.y + e2.z * q.z) * inv;

        if (tHit > EPS)
        {
            outDist = tHit;
            return true;
        }

        return false;
    }

    static void InsertTri(int idx, const ShadowTri& tri)
    {
        int minX = Cell(std::min({ tri.a.x, tri.b.x, tri.c.x }));
        int minY = Cell(std::min({ tri.a.y, tri.b.y, tri.c.y }));
        int minZ = Cell(std::min({ tri.a.z, tri.b.z, tri.c.z }));

        int maxX = Cell(std::max({ tri.a.x, tri.b.x, tri.c.x }));
        int maxY = Cell(std::max({ tri.a.y, tri.b.y, tri.c.y }));
        int maxZ = Cell(std::max({ tri.a.z, tri.b.z, tri.c.z }));

        for (int z = minZ; z <= maxZ; z++)
            for (int y = minY; y <= maxY; y++)
                for (int x = minX; x <= maxX; x++)
                    g_grid[{x, y, z}].push_back(idx);
    }

    void MeshBaker::ClearShadowCasters()
    {
        g_tris.clear();
        g_grid.clear();
    }

    void MeshBaker::AddShadowCaster(const Mesh& mesh, const Mat4& model)
    {
        for (const SubMesh& part : mesh.parts)
        {
            if (part.vertexCount < 3)
                continue;

            const float* v = part.vertices.data();

            for (int i = 0; i + 2 < part.vertexCount; i += 3)
            {
                ShadowTri t;

                t.a = Transform(model, v[(i + 0) * 8 + 0], v[(i + 0) * 8 + 1], v[(i + 0) * 8 + 2]);
                t.b = Transform(model, v[(i + 1) * 8 + 0], v[(i + 1) * 8 + 1], v[(i + 1) * 8 + 2]);
                t.c = Transform(model, v[(i + 2) * 8 + 0], v[(i + 2) * 8 + 1], v[(i + 2) * 8 + 2]);

                int idx = (int)g_tris.size();
                g_tris.push_back(t);
                InsertTri(idx, t);
            }
        }
    }

    static bool Raycast(const Vec3& o, const Vec3& d, float maxDist)
    {
        if (g_tris.empty())
            return false;

        float closest = maxDist;

        int x = Cell(o.x);
        int y = Cell(o.y);
        int z = Cell(o.z);

        int stepX = (d.x > 0) ? 1 : -1;
        int stepY = (d.y > 0) ? 1 : -1;
        int stepZ = (d.z > 0) ? 1 : -1;

        // FIX 1: Guard against division by zero when a ray direction component
        // is zero (e.g. a light directly above/beside the surface). A zero
        // component means the ray never crosses that axis, so we set the
        // corresponding t-values to a large sentinel so they never "win" the
        // DDA comparison and the traversal stays on the correct axes.
        const float INF = 1e30f;
        const float DIR_EPS = 1e-9f;

        float tMaxX = (fabs(d.x) > DIR_EPS) ? ((x + (stepX > 0)) * CELL - o.x) / d.x : INF;
        float tMaxY = (fabs(d.y) > DIR_EPS) ? ((y + (stepY > 0)) * CELL - o.y) / d.y : INF;
        float tMaxZ = (fabs(d.z) > DIR_EPS) ? ((z + (stepZ > 0)) * CELL - o.z) / d.z : INF;

        float tDeltaX = (fabs(d.x) > DIR_EPS) ? CELL / fabs(d.x) : INF;
        float tDeltaY = (fabs(d.y) > DIR_EPS) ? CELL / fabs(d.y) : INF;
        float tDeltaZ = (fabs(d.z) > DIR_EPS) ? CELL / fabs(d.z) : INF;

        float t = 0.0f;

        while (t < closest)
        {
            auto it = g_grid.find({ x,y,z });
            if (it != g_grid.end())
            {
                for (int idx : it->second)
                {
                    float hitDist;
                    if (RayTriDist(o, d, g_tris[idx], hitDist))
                    {
                        if (hitDist > 0.001f && hitDist < closest)
                            closest = hitDist;
                    }
                }
            }

            if (tMaxX < tMaxY)
            {
                if (tMaxX < tMaxZ)
                {
                    x += stepX;
                    t = tMaxX;
                    tMaxX += tDeltaX;
                }
                else
                {
                    z += stepZ;
                    t = tMaxZ;
                    tMaxZ += tDeltaZ;
                }
            }
            else
            {
                if (tMaxY < tMaxZ)
                {
                    y += stepY;
                    t = tMaxY;
                    tMaxY += tDeltaY;
                }
                else
                {
                    z += stepZ;
                    t = tMaxZ;
                    tMaxZ += tDeltaZ;
                }
            }
        }

        return closest < maxDist;
    }

    void MeshBaker::BakeMesh(
        Mesh& mesh,
        const Mat4& model,
        const std::vector<BakedLight>& lights)
    {
        for (SubMesh& part : mesh.parts)
        {
            float* v = part.vertices.data();

            
            

            std::vector<Vec3> smoothNorm(part.vertexCount, { 0,0,0 });

            // Quantise float coords to integer keys so vertices at the same
            // position (e.g. duplicated by the loader for UV seams) hash to
            // the same bucket.  QUANT = 1000 means positions within 0.001
            // units are treated as identical.
            const float QUANT = 1000.0f;
            auto quantise = [&](float x) -> int32_t {
                return (int32_t)std::floor(x * QUANT + 0.5f);
                };

            struct PosKey {
                int32_t x, y, z;
                bool operator==(const PosKey& o) const {
                    return x == o.x && y == o.y && z == o.z;
                }
            };
            struct PosKeyHash {
                size_t operator()(const PosKey& k) const {
                    uint32_t h = 2166136261u;
                    h = (h ^ (uint32_t)k.x) * 16777619u;
                    h = (h ^ (uint32_t)k.y) * 16777619u;
                    h = (h ^ (uint32_t)k.z) * 16777619u;
                    return (size_t)h;
                }
            };

            // Pass 1a — one pass over triangles, accumulate area-weighted face
            // normals into the map keyed by quantised world-space position.
            // O(n) instead of the previous O(n^2) inner loop.
            std::unordered_map<PosKey, Vec3, PosKeyHash> accumMap;
            accumMap.reserve(part.vertexCount);

            for (int tri = 0; tri < part.vertexCount; tri += 3)
            {
                int b0 = (tri + 0) * 8;
                int b1 = (tri + 1) * 8;
                int b2 = (tri + 2) * 8;

                Vec3 p0 = Transform(model, v[b0 + 0], v[b0 + 1], v[b0 + 2]);
                Vec3 p1 = Transform(model, v[b1 + 0], v[b1 + 1], v[b1 + 2]);
                Vec3 p2 = Transform(model, v[b2 + 0], v[b2 + 1], v[b2 + 2]);

                Vec3 e1{ p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };
                Vec3 e2{ p2.x - p0.x, p2.y - p0.y, p2.z - p0.z };

                // Unnormalised — magnitude = 2x triangle area, giving
                // automatic area-weighted averaging.
                Vec3 fn{
                    e1.y * e2.z - e1.z * e2.y,
                    e1.z * e2.x - e1.x * e2.z,
                    e1.x * e2.y - e1.y * e2.x
                };

                if (fn.x * fn.x + fn.y * fn.y + fn.z * fn.z < 1e-12f) continue;

                Vec3 pts[3] = { p0, p1, p2 };
                for (int k = 0; k < 3; k++)
                {
                    PosKey key{
                        quantise(pts[k].x),
                        quantise(pts[k].y),
                        quantise(pts[k].z)
                    };
                    Vec3& acc = accumMap[key];
                    acc.x += fn.x;
                    acc.y += fn.y;
                    acc.z += fn.z;
                }
            }

            // Pass 1b — one pass over vertices, look up and normalise.
            for (int i = 0; i < part.vertexCount; i++)
            {
                Vec3 pw = Transform(model, v[i * 8 + 0], v[i * 8 + 1], v[i * 8 + 2]);
                PosKey key{ quantise(pw.x), quantise(pw.y), quantise(pw.z) };

                auto it = accumMap.find(key);
                if (it == accumMap.end()) continue;

                const Vec3& acc = it->second;
                float len = sqrtf(acc.x * acc.x + acc.y * acc.y + acc.z * acc.z);
                if (len > 1e-6f)
                {
                    smoothNorm[i] = { acc.x / len, acc.y / len, acc.z / len };
                }
            }

            // ----------------------------------------------------------------
            // PASS 2 — shade each vertex using its smooth normal.
            // ----------------------------------------------------------------

            for (int i = 0; i < part.vertexCount; i++)
            {
                int base = i * 8;

                Vec3 p = Transform(model, v[base + 0], v[base + 1], v[base + 2]);
                Vec3 norm = smoothNorm[i];

                float r = 0.03f;
                float g = 0.03f;
                float b = 0.03f;

                for (const BakedLight& L : lights)
                {
                    Vec3 toL{
                        L.position.x - p.x,
                        L.position.y - p.y,
                        L.position.z - p.z
                    };

                    float distSq = toL.x * toL.x + toL.y * toL.y + toL.z * toL.z;
                    if (distSq > L.range * L.range)
                        continue;

                    float dist = sqrtf(distSq);
                    if (dist < 0.0001f) continue;

                    Vec3 dir{
                        toL.x / dist,
                        toL.y / dist,
                        toL.z / dist
                    };

                    // Back-face cull using the smooth normal.
                    float nDotL = norm.x * dir.x + norm.y * dir.y + norm.z * dir.z;
                    if (nDotL <= 0.0f) continue;

                    Vec3 origin{
                        p.x + dir.x * BIAS,
                        p.y + dir.y * BIAS,
                        p.z + dir.z * BIAS
                    };

                    float shadow = Raycast(origin, dir, dist) ? SHADOW : 1.0f;

                    float t = dist / L.range;
                    if (t > 1.0f) t = 1.0f;

                    float atten = powf(1.0f - t, L.falloff);
                    float diffuse = nDotL * atten;

                    r += L.color.x * L.intensity * diffuse * shadow;
                    g += L.color.y * L.intensity * diffuse * shadow;
                    b += L.color.z * L.intensity * diffuse * shadow;
                }

                v[base + 5] = (r > 1) ? 1 : r;
                v[base + 6] = (g > 1) ? 1 : g;
                v[base + 7] = (b > 1) ? 1 : b;
            }
        }
    }
}