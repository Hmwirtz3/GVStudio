

#include "Animation/Importer/MeshImporter.h"

#include "3rdParty/cgltf/cgltf.h"

#include <iostream>

static void ReadVec2(cgltf_accessor * accessor, cgltf_size index, float* out)
{
    cgltf_accessor_read_float(accessor, index, out, 2);
}

static void ReadVec3(cgltf_accessor* accessor, cgltf_size index, float* out)
{
    cgltf_accessor_read_float(accessor, index, out, 3);
}

bool MeshImporter::ImportMeshes(cgltf_data* data, std::vector<ImportedMesh>& meshes)
{
    meshes.clear();

    for (cgltf_size meshIndex = 0; meshIndex < data->meshes_count; ++meshIndex)
    {
        cgltf_mesh* mesh = &data->meshes[meshIndex];

        ImportedMesh importedMesh;

        if (mesh->name)
            importedMesh.name = mesh->name;
        else
            importedMesh.name = "UnnamedMesh";

        for (cgltf_size primitiveIndex = 0; primitiveIndex < mesh->primitives_count; ++primitiveIndex)
        {
            cgltf_primitive* primitive = &mesh->primitives[primitiveIndex];

            cgltf_accessor* positionAccessor = nullptr;
            cgltf_accessor* normalAccessor = nullptr;
            cgltf_accessor* uvAccessor = nullptr;

            for (cgltf_size attributeIndex = 0; attributeIndex < primitive->attributes_count; ++attributeIndex)
            {
                cgltf_attribute* attribute = &primitive->attributes[attributeIndex];

                if (attribute->type == cgltf_attribute_type_position)
                    positionAccessor = attribute->data;

                else if (attribute->type == cgltf_attribute_type_normal)
                    normalAccessor = attribute->data;

                else if (attribute->type == cgltf_attribute_type_texcoord)
                    uvAccessor = attribute->data;
            }

            if (!positionAccessor)
            {
                std::cout << "Mesh missing POSITION accessor\n";
                continue;
            }

            uint32_t baseVertex = (uint32_t)importedMesh.vertices.size();

            for (cgltf_size vertexIndex = 0; vertexIndex < positionAccessor->count; ++vertexIndex)
            {
                ImportedVertex vertex = {};

                ReadVec3(positionAccessor, vertexIndex, vertex.position);

                if (normalAccessor)
                    ReadVec3(normalAccessor, vertexIndex, vertex.normal);

                if (uvAccessor)
                    ReadVec2(uvAccessor, vertexIndex, vertex.uv);

                importedMesh.vertices.push_back(vertex);
            }

            if (primitive->indices)
            {
                cgltf_accessor* indexAccessor = primitive->indices;

                for (cgltf_size index = 0; index < indexAccessor->count; ++index)
                {
                    uint32_t value = (uint32_t)cgltf_accessor_read_index(indexAccessor, index);

                    importedMesh.indices.push_back(baseVertex + value);
                }
            }
        }

        std::cout << "Imported Mesh: " << importedMesh.name << "\n";
        std::cout << "Vertices: " << importedMesh.vertices.size() << "\n";
        std::cout << "Indices: " << importedMesh.indices.size() << "\n";

        meshes.push_back(importedMesh);
    }

    return true;
}

