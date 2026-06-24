// AnimationPlayer.cpp

#include "Animation/AnimationPlayer.h"

AnimationPlayer::AnimationPlayer()
{
    m_clip = nullptr;
    m_currentTime = 0.0f;
    m_loop = true;
    m_playing = false;
}

void AnimationPlayer::Play(AnimationClip* clip, bool loop)
{
    m_clip = clip;
    m_currentTime = 0.0f;
    m_loop = loop;
    m_playing = true;
}

void AnimationPlayer::Stop()
{
    m_playing = false;
    m_currentTime = 0.0f;
}

void AnimationPlayer::Update(float deltaTime)
{
    if (!m_playing)
        return;

    if (!m_clip)
        return;

    m_currentTime += deltaTime;

    if (m_currentTime > m_clip->duration)
    {
        if (m_loop)
        {
            while (m_currentTime > m_clip->duration)
                m_currentTime -= m_clip->duration;
        }
        else
        {
            m_currentTime = m_clip->duration;
            m_playing = false;
        }
    }
}

bool AnimationPlayer::IsPlaying() const
{
    return m_playing;
}

float AnimationPlayer::GetCurrentTime() const
{
    return m_currentTime;
}

AnimationClip* AnimationPlayer::GetClip() const
{
    return m_clip;
}