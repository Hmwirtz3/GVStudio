#include "GVFramework/Scene/SceneObject.h"

class LogicUnitInspectorPanel
{

private:

	bool m_isVisible = true;

public: 

	void Draw(SceneObject* selectedObject);
	void SetVisible(bool visible);
	bool IsVisible() const;
};