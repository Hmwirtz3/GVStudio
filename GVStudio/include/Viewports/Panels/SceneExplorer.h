#pragma once

#include <cstddef>


#include "GVFramework/Scene/SceneManager.h"
#include "GVFramework/Scene/SceneObject.h"

class SceneManager;

class SceneExplorer
{
public:
    // Main draw entry
    void Draw(
        GV_State& state,
        SceneManager& sceneManager,
        SceneObject*& selectedObject,
        SceneFolder*& selectedFolder);

    void SetVisible(bool visible);
    bool IsVisible() const;

private:
    void DrawSceneFoldersRecursive(
        SceneFolder& folder,
        SceneManager& sceneManager,
        SceneObject*& selectedObject,
        SceneFolder*& selectedFolder);

private:
    // UI state only
    bool m_isVisible = true;

    char m_renameBuffer[256] = {};

    SceneFolder* m_newFolderParent = nullptr;
    SceneFolder* m_folderPendingDelete = nullptr;
    SceneFolder* m_folderRenaming = nullptr;

    SceneObject* m_objectPendingDelete = nullptr;
    SceneObject* m_objectRenaming = nullptr;
};
