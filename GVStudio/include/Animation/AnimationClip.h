// AnimationClip.h

#pragma once

#include <string>
#include <vector>

#include "MiniMath/MiniMath.h"

struct AnimationVec3Key
{
    float time;
    Vec3 value;
};

struct AnimationQuatKey
{
    float time;
    Quat value;
};

struct AnimationNodeTrack
{
    int nodeIndex;

    std::vector<AnimationVec3Key> translationKeys;
    std::vector<AnimationQuatKey> rotationKeys;
    std::vector<AnimationVec3Key> scaleKeys;
};

class AnimationClip
{
public:
    std::string name;

    float duration;

    std::vector<AnimationNodeTrack> tracks;
};