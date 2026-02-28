#pragma once

class LogicUnitRegistry;
struct GV_State;

class LogicUnitRegistryTab
{
public:
    void Draw(GV_State& state, LogicUnitRegistry& registry);
    void SetVisible(bool visible);
    bool IsVisible() const;

private:
    bool m_isVisible = true;
};
