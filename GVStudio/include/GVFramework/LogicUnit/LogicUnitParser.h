#pragma once

#include <vector>
#include <string>

#include "GVFramework/LogicUnit/LogicUnit.h"

class LogicUnitParser
{
public:
    static std::vector<GV_Logic_Unit> ParseFile(const std::string& filename);

private:
    static std::string Trim(const std::string& str);
    static std::string StripQuotes(const std::string& s);
};
