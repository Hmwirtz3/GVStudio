#pragma once

#include "Viewports/Toolbars/MainToolbar/FileTab.h"

struct GV_State;
class SceneManager;

class Toolbar
{
public:
    void Draw(GV_State& state, SceneManager& sceneManager);

private:
    FileTab m_fileTab;
};