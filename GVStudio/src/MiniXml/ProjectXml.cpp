#include "MiniXml/ProjectXml.h"
#include "MiniXml/MiniXml.h"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

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
        size_t slash = path.find_last_of("\\/");
        if (slash == std::string::npos)
            return path;

        return path.substr(slash + 1);
    }
}

bool ProjectXml::LoadProjectFromXml(GV_Project_Info& project, const std::string& xmlPath)
{
    std::cout << "\n[ProjectXml] Loading project: " << xmlPath << "\n";

    XmlNode root;

    if (!XmlLoadFromFile(xmlPath, root))
    {
        std::cout << "[ProjectXml] Failed to load XML file.\n";
        return false;
    }

    if (root.name != "Project")
    {
        std::cout << "[ProjectXml] Invalid root node: " << root.name << "\n";
        return false;
    }

    project = GV_Project_Info();
    project.projectPath = xmlPath;

    project.projectRoot =
        fs::path(xmlPath).parent_path().string();


    for (const XmlNode& child : root.children)
    {
        if (child.name == "Name")
        {
            project.projectName = child.text;
            std::cout << "[ProjectXml] Project Name: "
                << project.projectName << "\n";
        }
        else if (child.name == "Paths")
        {
            std::cout << "[ProjectXml] Reading Paths...\n";

            for (const XmlNode& pathNode : child.children)
            {
                if (pathNode.name == "DataFolder")
                {
                    project.dataFolder = pathNode.text;
                    std::cout << "  DataFolder: "
                        << project.dataFolder << "\n";
                }
                else if (pathNode.name == "ResourceFolder")
                {
                    project.resourceFolder = pathNode.text;
                    std::cout << "  ResourceFolder: "
                        << project.resourceFolder << "\n";
                }
                else if (pathNode.name == "SourceFolder")
                {
                    project.sourceFolder = pathNode.text;
                    std::cout << "  SourceFolder: "
                        << project.sourceFolder << "\n";
                }
            }
        }
        else if (child.name == "Scenes")
        {
            std::cout << "[ProjectXml] Reading Scenes...\n";

            for (const XmlNode& sceneNode : child.children)
            {
                if (sceneNode.name == "Scene")
                {
                    const XmlAttribute* pathAttr =
                        FindAttr(sceneNode, "path");

                    if (pathAttr)
                    {
                        GV_Scene_Info scene;
                        scene.scenePath = pathAttr->value;
                        scene.sceneName =
                            GetFileNameFromPath(scene.scenePath);

                        project.scenes.push_back(scene);

                        std::cout << "  Scene Loaded:\n";
                        std::cout << "    Path: "
                            << scene.scenePath << "\n";
                        std::cout << "    Name: "
                            << scene.sceneName << "\n";
                    }
                }
            }
        }
        else if (child.name == "StartupScene")
        {
            project.startupScene = child.text;
            std::cout << "[ProjectXml] StartupScene: "
                << project.startupScene << "\n";
        }
    }

    std::cout << "[ProjectXml] Total Scenes: "
        << project.scenes.size() << "\n";

    std::cout << "[ProjectXml] Project load complete.\n\n";

    return true;
}

bool ProjectXml::SaveProjectToXml(
    const GV_Project_Info& project,
    const std::string& xmlPath)
{
    std::cout << "\n[ProjectXml] Saving project: "
        << xmlPath << "\n";

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

            std::cout << "  Saving Scene:\n";
            std::cout << "    Path: "
                << scene.scenePath << "\n";
        }

        root.children.push_back(scenesNode);
    }

    {
        XmlNode startupNode;
        startupNode.name = "StartupScene";
        startupNode.text = project.startupScene;
        root.children.push_back(startupNode);

        std::cout << "  StartupScene: "
            << project.startupScene << "\n";
    }

    if (!XmlSaveToFile(xmlPath, root))
    {
        std::cout << "[ProjectXml] Failed to save project file.\n";
        return false;
    }

    std::cout << "[ProjectXml] Project saved successfully.\n\n";

    return true;
}
