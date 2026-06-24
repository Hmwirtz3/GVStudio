#pragma once

#include <string>
#include <vector>

struct cgltf_data;
struct cgltf_node;

struct ImportedVec3Key
{
    float time;
    float value[3];
};

struct ImportedQuatKey
{
    float time;
    float value[4];
};

struct ImportedNodeAnimation
{
    int nodeIndex;
    std::vector<ImportedVec3Key> translationKeys;
    std::vector<ImportedQuatKey> rotationKeys;
    std::vector<ImportedVec3Key> scaleKeys;
};

struct ImportedAnimation
{
    std::string name;
    float duration;
    std::vector<ImportedNodeAnimation> nodeAnimations;
};

class AnimationImporter
{
public:
    static bool ImportAnimations(cgltf_data* data, std::vector<ImportedAnimation>& animations);

private:
    static int GetNodeIndex(cgltf_data* data, cgltf_node* node);
    static ImportedNodeAnimation* FindOrCreateNodeAnimation(ImportedAnimation& animation, int nodeIndex);
};