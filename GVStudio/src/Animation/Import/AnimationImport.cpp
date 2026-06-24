#include "Animation/Importer/AnimationImporter.h"

#include "3rdParty/cgltf/cgltf.h"

#include <iostream>

int AnimationImporter::GetNodeIndex(cgltf_data* data, cgltf_node* node)
{
    if (!data || !node)
        return -1;

    for (cgltf_size i = 0; i < data->nodes_count; ++i)
    {
        if (&data->nodes[i] == node)
            return (int)i;
    }

    return -1;
}

ImportedNodeAnimation* AnimationImporter::FindOrCreateNodeAnimation(ImportedAnimation& animation, int nodeIndex)
{
    for (size_t i = 0; i < animation.nodeAnimations.size(); ++i)
    {
        if (animation.nodeAnimations[i].nodeIndex == nodeIndex)
            return &animation.nodeAnimations[i];
    }

    ImportedNodeAnimation nodeAnimation;
    nodeAnimation.nodeIndex = nodeIndex;
    animation.nodeAnimations.push_back(nodeAnimation);

    return &animation.nodeAnimations.back();
}

bool AnimationImporter::ImportAnimations(cgltf_data* data, std::vector<ImportedAnimation>& animations)
{
    animations.clear();

    if (!data)
        return false;

    for (cgltf_size animationIndex = 0; animationIndex < data->animations_count; ++animationIndex)
    {
        cgltf_animation* cgltfAnimation = &data->animations[animationIndex];

        ImportedAnimation animation;
        animation.duration = 0.0f;

        if (cgltfAnimation->name)
            animation.name = cgltfAnimation->name;
        else
            animation.name = "UnnamedAnimation";

        for (cgltf_size channelIndex = 0; channelIndex < cgltfAnimation->channels_count; ++channelIndex)
        {
            cgltf_animation_channel* channel = &cgltfAnimation->channels[channelIndex];

            if (!channel->target_node || !channel->sampler)
                continue;

            cgltf_animation_sampler* sampler = channel->sampler;

            if (!sampler->input || !sampler->output)
                continue;

            int nodeIndex = GetNodeIndex(data, channel->target_node);

            if (nodeIndex < 0)
                continue;

            ImportedNodeAnimation* nodeAnimation = FindOrCreateNodeAnimation(animation, nodeIndex);

            cgltf_accessor* inputAccessor = sampler->input;
            cgltf_accessor* outputAccessor = sampler->output;

            for (cgltf_size keyIndex = 0; keyIndex < inputAccessor->count; ++keyIndex)
            {
                float time = 0.0f;
                cgltf_accessor_read_float(inputAccessor, keyIndex, &time, 1);

                if (time > animation.duration)
                    animation.duration = time;

                if (channel->target_path == cgltf_animation_path_type_translation)
                {
                    ImportedVec3Key key;
                    key.time = time;

                    key.value[0] = 0.0f;
                    key.value[1] = 0.0f;
                    key.value[2] = 0.0f;

                    cgltf_accessor_read_float(outputAccessor, keyIndex, key.value, 3);

                    nodeAnimation->translationKeys.push_back(key);
                }
                else if (channel->target_path == cgltf_animation_path_type_rotation)
                {
                    ImportedQuatKey key;
                    key.time = time;

                    key.value[0] = 0.0f;
                    key.value[1] = 0.0f;
                    key.value[2] = 0.0f;
                    key.value[3] = 1.0f;

                    cgltf_accessor_read_float(outputAccessor, keyIndex, key.value, 4);

                    nodeAnimation->rotationKeys.push_back(key);
                }
                else if (channel->target_path == cgltf_animation_path_type_scale)
                {
                    ImportedVec3Key key;
                    key.time = time;

                    key.value[0] = 1.0f;
                    key.value[1] = 1.0f;
                    key.value[2] = 1.0f;

                    cgltf_accessor_read_float(outputAccessor, keyIndex, key.value, 3);

                    nodeAnimation->scaleKeys.push_back(key);
                }
            }
        }

        std::cout << "Animation Imported: " << animation.name << "\n";
        std::cout << "Duration: " << animation.duration << "\n";
        std::cout << "Animated Nodes: " << animation.nodeAnimations.size() << "\n";

        animations.push_back(animation);
    }

    return true;
}