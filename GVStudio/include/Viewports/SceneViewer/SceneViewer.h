#include "GVFramework/Scene/SceneManager.h"
#include "Viewports/SceneViewer/EditorCamera.h"
#include "Renderer/Renderer.h"

class SceneViewer
{
public:

	void Resize(int width, int height);
	void Update();

	void Render(SceneFolder& scene);

	unsigned int GetColorTexture() const;

	
private:
	int m_width = 1;
	int m_height = 1;

	EditorCamera m_camera;
	Renderer m_renderer;
};