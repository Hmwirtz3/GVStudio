#pragma once

#include <string>
#include <vector>

enum class EditorMode
{
	NoProject,
	ProjectOpen,
	OpenRecent,

};

struct GV_Project_Info
{
	std::string projectPath;
	std::string projectName;
	std::string dataFolder;
	std::string resourceFolder;
	std::string sourceFolder;
};

struct GV_Scene_Info
{
	std::string scenePath;
	std::string sceneName;
};

struct GV_State
{
	EditorMode mode = EditorMode::NoProject;
	GV_Project_Info project;
	GV_Scene_Info currentScene;
	std::vector<GV_Scene_Info> availableScenes;
	bool showStartupDialog = true;
	bool quitRequested = false;
	bool openRecent = false;

	GV_Scene_Info recentScene;
};

class GV_STUDIO
{
public:
	int RUN();

private:
	void ShowStartupDialog(GV_State);
	


};