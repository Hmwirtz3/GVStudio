#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

#pragma pack(push, 1)
struct GV_ChunkHeader {
    uint32_t type;
    uint32_t size;
    uint32_t version;
};
#pragma pack(pop)

enum GV_ChunkType : uint32_t
{
    
    GV_CHUNK_STRUCT = 0x0001,
    GV_CHUNK_STRING = 0x0002,
    GV_CHUNK_EXTENSION = 0x0003,
    GV_CHUNK_LOGIC_UNIT = 0x0004,
    GV_CHUNK_SCENE_OBJECT = 0x0024,


    
    GV_CHUNK_CAMERA = 0x0005,
    GV_CHUNK_TEXTURE = 0x0006,
    GV_CHUNK_MATERIAL = 0x0007,
    GV_CHUNK_MATERIAL_LIST = 0x0008,
    GV_CHUNK_ATOMIC_SECT = 0x0009,
    GV_CHUNK_PLANE_SECT = 0x000A,

    
    GV_CHUNK_WORLD = 0x000B, 
    GV_CHUNK_SPLINE = 0x000C,
    GV_CHUNK_MATRIX = 0x000D,
    GV_CHUNK_FRAME_LIST = 0x000E,
    GV_CHUNK_GEOMETRY = 0x000F,
    GV_CHUNK_CLUMP = 0x0010,
    GV_CHUNK_HEIGHTMAP = 0x0019,
    GV_CHUNK_STATIC_MESH = 0x0013,

    
    GV_CHUNK_LIGHT = 0x0011,
    GV_CHUNK_UNICODE_STRING = 0x0012,
    GV_CHUNK_ATOMIC = 0x0014,
    GV_CHUNK_TEXTURE_NATIVE = 0x0015,
    GV_CHUNK_TEXDICTIONARY = 0x0016,
    GV_CHUNK_ANIMDATABASE = 0x0017,
    GV_CHUNK_IMAGE = 0x0018,

    
    GV_CHUNK_WORLD_SECTOR = 0x0020,
    GV_CHUNK_VERT_NORMALS = 0x0021,
    GV_CHUNK_LIGHT_ATOMICS = 0x0022,
    GV_CHUNK_COLLISION_MESH = 0x0023,

    
    GV_CHUNK_GEOMETRY_LIST = 0x001A,
    GV_CHUNK_SKIN_PLG = 0x0116,
    GV_CHUNK_HANIM_PLG = 0x011E,

    
    GV_CHUNK_BIN_MESH_PLG = 0x0500,
    GV_CHUNK_NATIVEDATA_PLG = 0x0501,
    GV_CHUNK_MORPH_PLG = 0x1050,

    // ───────────── Misc / Game Specific ─────────────
    GV_CHUNK_PSA = 0x0600, // Particle Stream Archive (GTA)
    GV_CHUNK_TOOL_PLG = 0x1000, // Used by 3ds Max toolchain
    GV_CHUNK_USERDATA_PLG = 0x1100,
    GV_CHUNK_RIGHTTORENDER_PLG = 0x1110,

    // ───────────── Terminator ─────────────
    GV_CHUNK_TERMINATOR = 0xFFFFFFFF,

    GV_CHUNK_UNKNOWN = 0x12345678
};




void WriteChunk(std::ofstream& out, uint32_t type, uint32_t version, const std::vector<char>& data);
void AppendChunk(std::vector<char>& dest, uint32_t type, uint32_t version, const std::vector<char>& payload);
