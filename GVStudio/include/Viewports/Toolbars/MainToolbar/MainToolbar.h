#pragma once

#include "Viewports/Toolbars/MainToolbar/FileTab.h"
#include "Viewports/Toolbars/MainToolbar/ExporterTab.h"

struct GV_State;
class SceneManager;

class Toolbar
{
public:
    void Draw(GV_State& state, SceneManager& sceneManager);
    bool ConsumeExportRequest();

private:
    FileTab m_fileTab;
    ExportTab m_exportTab;
};