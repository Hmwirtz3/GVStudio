//#include "GVFramework/Scene/SceneManager.h"
#include "Viewports/SceneViewer/SceneViewer.h"

class ViewportPanel
{
public:

	void Draw(SceneFolder& scene, const std::string& resourceRoot);

private:

	SceneViewer m_viewer;
	int m_width = 0;
	int m_height = 0;
};