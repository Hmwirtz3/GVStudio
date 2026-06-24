// AnimationPlayer.h

#pragma once

#include <vector>

#include "Animation/AnimationClip.h"

class AnimationPlayer
{
public:
    AnimationPlayer();

    void Play(AnimationClip* clip, bool loop = true);

    void Stop();

    void Update(float deltaTime);

    bool IsPlaying() const;

    float GetCurrentTime() const;

    AnimationClip* GetClip() const;

private:
    AnimationClip* m_clip;

    float m_currentTime;

    bool m_loop;

    bool m_playing;
};