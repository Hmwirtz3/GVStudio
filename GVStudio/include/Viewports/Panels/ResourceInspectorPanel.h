#pragma once

#include "Viewports/Panels/ResourceDatabaseTab.h"
#include "Viewports/Panels/LogicUnitRegistryTab.h"

class LogicUnitRegistry;
class ResourceDatabase;
struct GV_State;

class ResourceInspectorPanel
{
public:
    void Draw(GV_State& state,
        LogicUnitRegistry& registry,
        ResourceDatabase& resourceDatabase);

private: 
    void SetVisible(bool visible);
    bool IsVisible() const;

private:
    ResourceDatabaseTab m_resourceTab;
    LogicUnitRegistryTab m_logicUnitTab;

    bool m_isVisible = true;
};

