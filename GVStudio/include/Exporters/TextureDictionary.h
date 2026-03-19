#pragma once

#include <vector>
#include <string>

#include "Exporters/Exporter.h"
#include "Exporters/ExportContext.h"

// ============================================================
// Texture Dictionary Exporter
// ============================================================

struct TextureDictionary
{
    // empty — uses context only
};


// ============================================================
// Specializations (FUNCTION ONLY — NOT STRUCT)
// ============================================================

template<>
void GV_Exporter<TextureDictionary>::Build(
    const TextureDictionary& dict,
    GV_ChunkExporter& exporter,
    const GV_ExportContext& ctx);

template<>
uint32_t GV_Exporter<TextureDictionary>::GetChunkType();


// ============================================================
// Image Loading + Conversion
// ============================================================

bool LoadImageRGBA(
    const std::string& path,
    int& width,
    int& height,
    std::vector<uint8_t>& pixels);

uint16_t ConvertRGBAto565(uint8_t r, uint8_t g, uint8_t b);