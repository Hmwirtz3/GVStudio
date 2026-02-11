#include "GVStudio/GVStudio.h"
#include <string>
#include <vector>
#include <filesystem>



GV_STUDIO::GV_STUDIO()
    : m_rootFolder(),
    m_logicRegistry(),
    m_sceneManager(m_rootFolder, m_logicRegistry),
    m_assetDataBase()
{
}


int GV_STUDIO::RUN()
{
	return 0;
}

void GV_STUDIO::ShowStartupDialog(GV_State)
{
}

