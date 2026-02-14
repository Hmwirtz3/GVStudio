#include "MiniXml/ProjectXml.h"
#include "MiniXml/MiniXml.h"

#include <iostream>

namespace
{
	const XmlAttribute* FindAttr(const XmlNode& node, const std::string& name)
	{
		for (const XmlAttribute& a : node.attributes)
		{
			if (a.name == name)
				return &a;
			
		}
		return nullptr;
	}


	std::string GetFileNameFromPath(const std::string& path)
	{
		size_t slash = path.find_last_of("\//");

		if (slash == std::string::npos)
		{
			return path;
		}

		return path.substr(slash + 1);

	}
}




bool ProjectXml::LoadProjectFromXml(GV_Project_Info& project, const std::string& xmlPath)
{
	XmlNode root;

	if (!XmlLoadFromFile(xmlPath, root))
	{
		std::cout << "Failed to load project file: " << xmlPath << "\n";
		return false;
	}
	

	if (root.name != "Project")
	{
		std::cout << "Invalid project file format. \n";
			return false;
	}

	project = GV_Project_Info();
	project.projectPath = xmlPath;

	for (const XmlNode& child : root.children)
	{
		if (child.name == "Name")
		{
			project.projectName == child.text;

		}
		else if (child.name == "Paths")
		{
			for (const XmlNode& pathNode : child.children)
			{
				if (pathNode.name == "DataFolder")
				{
					project.dataFolder == pathNode.text;
				}
				else if (pathNode.name == "ResourceFolder")
				{
					project.resourceFolder == pathNode.text;
				}
				else if (pathNode.name == "SourceFolder")
				{
					project.sourceFolder == pathNode.text;
				}
			}

		}

		else if (child.name == "Scenes")
		{
			for (const XmlNode& sceneNode : child.children)
			{
				if (sceneNode.name == "Scene")
				{
					const XmlAttribute* pathAttr = FindAttr(sceneNode, "path");

					if (pathAttr)
					{
						GV_Scene_Info scene;
						scene.scenePath = pathAttr->value;
						scene.sceneName = GetFileNameFromPath(scene.scenePath);

						project.scenes.push_back(scene);
					}
				}
			}
		}
		else if (child.name == "StartupScene")
		{
			project.startupScene = child.text;
		}
	}
	return true;
}




bool ProjectXml::SaveProjectToXml(const GV_Project_Info& project,  const std::string& xmlPath)
{
	XmlNode root;
	root.name = "Project";

	
	XmlAttribute versionAttr;
	versionAttr.name = "version";
	versionAttr.value = "1";
	root.attributes.push_back(versionAttr);

	
	{
		XmlNode nameNode;
		nameNode.name = "Name";
		nameNode.text = project.projectName;
		root.children.push_back(nameNode);
	}


	{
		XmlNode pathsNode;
		pathsNode.name = "Paths";

		XmlNode dataNode;
		dataNode.name = "DataFolder";
		dataNode.text = project.dataFolder;
		pathsNode.children.push_back(dataNode);

		XmlNode resourceNode;
		resourceNode.name = "ResourceFolder";
		resourceNode.text = project.resourceFolder;
		pathsNode.children.push_back(resourceNode);

		XmlNode sourceNode;
		sourceNode.name = "SourceFolder";
		sourceNode.text = project.sourceFolder;
		pathsNode.children.push_back(sourceNode);

		root.children.push_back(pathsNode);
	}


	{
		XmlNode scenesNode;
		scenesNode.name = "Scenes";

		for (const GV_Scene_Info& scene : project.scenes)
		{
			XmlNode sceneNode;
			sceneNode.name = "Scene";

			XmlAttribute pathAttr;
			pathAttr.name = "path";
			pathAttr.value = scene.scenePath;

			sceneNode.attributes.push_back(pathAttr);
			scenesNode.children.push_back(sceneNode);
		}

		root.children.push_back(scenesNode);
	}

	
	{
		XmlNode startupNode;
		startupNode.name = "StartupScene";
		startupNode.text = project.startupScene;
		root.children.push_back(startupNode);
	}

	if (!XmlSaveToFile(xmlPath, root))
	{
		std::cout << "Failed to save project file: " << xmlPath << "\n";
		return false;
	}

	return true;
}

