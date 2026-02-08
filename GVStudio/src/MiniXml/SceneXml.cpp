#include "MiniXml/SceneXml.h"
#include "GVFramework/Scene/SceneObject.h"
#include "GVFramework/Scene/SceneManager.h"
#include "MiniXML/MiniXml.h"

#include <iostream>

#include <memory>
#include <string>

namespace
{
    const XmlAttribute* FindAttr(const XmlNode& node, const std::string& name)
    {
        for (const XmlAttribute& a : node.attributes)
            if (a.name == name)
                return &a;
        return nullptr;
    }

    void LoadFolder(const XmlNode& xmlFolder, SceneFolder& folder, SceneFolder* parent)
    {
        folder.parent = parent;

        if (const XmlAttribute* nameAttr = FindAttr(xmlFolder, "name"))
            folder.name = nameAttr->value;

        folder.objects.clear();
        folder.children.clear();

        for (const XmlNode& child : xmlFolder.children)
        {
            if (child.name == "Object")
            {
                auto obj = std::make_unique<SceneObject>();

                if (const XmlAttribute* n = FindAttr(child, "name"))
                    obj->name = n->value;

                if (const XmlAttribute* a = FindAttr(child, "asset"))
                    obj->assetPath = a->value;

                folder.objects.push_back(std::move(obj));
            }
            else if (child.name == "Folder")
            {
                auto sub = std::make_unique<SceneFolder>();
                LoadFolder(child, *sub, &folder);
                folder.children.push_back(std::move(sub));
            }
        }
    }

    void BuildFolderNode(const SceneFolder& folder, XmlNode& outNode)
    {
        outNode.name = "Folder";
        outNode.attributes.push_back({ "name", folder.name });

        for (const auto& obj : folder.objects)
        {
            XmlNode objNode;
            objNode.name = "Object";
            objNode.attributes.push_back({ "name", obj->name });

            if (!obj->assetPath.empty())
                objNode.attributes.push_back({ "asset", obj->assetPath });

            outNode.children.push_back(std::move(objNode));
        }

        for (const auto& sub : folder.children)
        {
            XmlNode subNode;
            BuildFolderNode(*sub, subNode);
            outNode.children.push_back(std::move(subNode));
        }
    }
}

namespace SceneXml
{
    bool LoadGScene(const std::string& path, SceneFolder& root)
    {
        XmlNode xml;
        if (!XmlLoadFromFile(path, xml))
            return false;

        if (xml.name != "Scene")
            return false;

        const XmlNode* rootFolderNode = nullptr;
        for (const XmlNode& child : xml.children)
        {
            if (child.name == "Folder")
            {
                rootFolderNode = &child;
                break;
            }
        }

        if (!rootFolderNode)
            return false;

        root.children.clear();
        root.objects.clear();

        LoadFolder(*rootFolderNode, root, nullptr);
        return true;
    }

    bool SaveGScene(const std::string& path, const SceneFolder& rootFolder)
    {
        XmlNode scene;
        scene.name = "Scene";

        XmlNode rootNode;
        BuildFolderNode(rootFolder, rootNode);
        scene.children.push_back(std::move(rootNode));

        return XmlSaveToFile(path, scene);
    }
}
