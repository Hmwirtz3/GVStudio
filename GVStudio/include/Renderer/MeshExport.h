#pragma once

#include "Renderer/Renderer.h"
#include "Exporters/ExportContext.h"

namespace GV
{
    GV_MeshData ConvertToExportMesh(const Mesh& mesh);
}