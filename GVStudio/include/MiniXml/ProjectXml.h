#pragma once

#include "GVStudio/GVStudio.h"

#include <string>

namespace ProjectXml
{
    bool LoadProjectFromXml(
        GV_Project_Info& project,
        const std::string& xmlPath);

    bool SaveProjectToXml(
        const GV_Project_Info& project,
        const std::string& xmlPath);
}
