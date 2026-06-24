
#include "Animation/Importer/GLTFImport.h"
#include "Animation/Importer/MeshImporter.h"
#include "Animation/Importer/AnimationImporter.h"

#define CGLTF_IMPLEMENTATION
#include "3rdParty/cgltf/cgltf.h"

#include <iostream>

GLTFImporter::GLTFImporter() : m_data(nullptr)
{
}

GLTFImporter::~GLTFImporter()
{
    if (m_data)
    {
        cgltf_free(m_data);
        m_data = nullptr;
    }
}

bool GLTFImporter::Import(const std::string& path)
{
    m_scene.nodes.clear();
    m_scene.meshes.clear();
    m_scene.animations.clear();

    if (m_data)
    {
        cgltf_free(m_data);
        m_data = nullptr;
    }

    cgltf_options options = {};

    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &m_data);

    if (result != cgltf_result_success)
    {
        std::cout << "Failed to parse glTF file\n";
        return false;
    }

    result = cgltf_load_buffers(&options, m_data, path.c_str());

    if (result != cgltf_result_success)
    {
        std::cout << "Failed to load glTF buffers\n";

        cgltf_free(m_data);
        m_data = nullptr;

        return false;
    }

    if (!MeshImporter::ImportMeshes(m_data, m_scene.meshes))
    {
        cgltf_free(m_data);
        m_data = nullptr;

        return false;
    }

    if (!AnimationImporter::ImportAnimations(m_data, m_scene.animations))
    {
        cgltf_free(m_data);
        m_data = nullptr;

        return false;
    }

    if (!ParseNodes())
    {
        cgltf_free(m_data);
        m_data = nullptr;

        return false;
    }

    cgltf_free(m_data);
    m_data = nullptr;

    return true;
}

const ImportedScene& GLTFImporter::GetScene() const
{
    return m_scene;
}

bool GLTFImporter::ParseNodes()
{
    for (cgltf_size i = 0; i < m_data->nodes_count; ++i)
    {
        cgltf_node* node = &m_data->nodes[i];

        ImportedNode importedNode;

        if (node->name)
            importedNode.name = node->name;
        else
            importedNode.name = "UnnamedNode";

        importedNode.parentIndex = GetNodeIndex(node->parent);
        importedNode.meshIndex = GetMeshIndex(node);

        importedNode.translation[0] = 0.0f;
        importedNode.translation[1] = 0.0f;
        importedNode.translation[2] = 0.0f;

        importedNode.rotation[0] = 0.0f;
        importedNode.rotation[1] = 0.0f;
        importedNode.rotation[2] = 0.0f;
        importedNode.rotation[3] = 1.0f;

        importedNode.scale[0] = 1.0f;
        importedNode.scale[1] = 1.0f;
        importedNode.scale[2] = 1.0f;

        if (node->has_translation)
        {
            importedNode.translation[0] = (float)node->translation[0];
            importedNode.translation[1] = (float)node->translation[1];
            importedNode.translation[2] = (float)node->translation[2];
        }

        if (node->has_rotation)
        {
            importedNode.rotation[0] = (float)node->rotation[0];
            importedNode.rotation[1] = (float)node->rotation[1];
            importedNode.rotation[2] = (float)node->rotation[2];
            importedNode.rotation[3] = (float)node->rotation[3];
        }

        if (node->has_scale)
        {
            importedNode.scale[0] = (float)node->scale[0];
            importedNode.scale[1] = (float)node->scale[1];
            importedNode.scale[2] = (float)node->scale[2];
        }

        m_scene.nodes.push_back(importedNode);

        std::cout << "Node Imported: " << importedNode.name << "\n";
        std::cout << "Parent Index: " << importedNode.parentIndex << "\n";
        std::cout << "Mesh Index: " << importedNode.meshIndex << "\n";
    }

    return true;
}

int GLTFImporter::GetNodeIndex(cgltf_node* node)
{
    if (!node)
        return -1;

    for (cgltf_size i = 0; i < m_data->nodes_count; ++i)
    {
        if (&m_data->nodes[i] == node)
            return (int)i;
    }

    return -1;
}

int GLTFImporter::GetMeshIndex(cgltf_node* node)
{
    if (!node || !node->mesh)
        return -1;

    for (cgltf_size i = 0; i < m_data->meshes_count; ++i)
    {
        if (&m_data->meshes[i] == node->mesh)
            return (int)i;
    }

    return -1;
}

