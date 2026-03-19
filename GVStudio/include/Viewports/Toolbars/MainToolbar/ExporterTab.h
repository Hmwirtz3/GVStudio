#pragma once

#include "GVFramework/Scene/SceneManager.h"

struct GV_State;

struct ExportTab
{
public:
    void Draw(GV_State& state, SceneManager& sceneManager);

    bool ConsumeExportRequest();

private:
    bool m_exportRequested = false;
};