#include "Database/LogicUnitRegistry.h"
#include "GVFramework/LogicUnit/LogicUnitParser.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void LogicUnitRegistry::Clear()
{
    m_units.clear();
    m_lookup.clear();
}

void LogicUnitRegistry::Register(const GV_Logic_Unit& unit)
{
    size_t index = m_units.size();
    m_units.push_back(unit);
    m_lookup[m_units.back().typeName] = index;

    std::cout << "[Registry] Registered Logic Unit: "
        << unit.typeName << "\n";
}

void LogicUnitRegistry::Register(GV_Logic_Unit&& unit)
{
    size_t index = m_units.size();
    m_units.push_back(std::move(unit));
    m_lookup[m_units.back().typeName] = index;

    std::cout << "[Registry] Registered Logic Unit: "
        << m_units.back().typeName << "\n";
}

const GV_Logic_Unit* LogicUnitRegistry::Find(const std::string& typeName) const
{
    auto it = m_lookup.find(typeName);
    if (it == m_lookup.end())
        return nullptr;

    return &m_units[it->second];
}

void LogicUnitRegistry::ParseFile(const std::string& filePath)
{
    LogicUnitParser parser;
    auto units = parser.ParseFile(filePath);

    for (auto& unit : units)
    {
        Register(std::move(unit));
    }
}

void LogicUnitRegistry::ParseFolder(const std::string& folderPath)
{
    std::cout << "\n[Registry] Parsing folder: "
        << folderPath << "\n";

    if (!fs::exists(folderPath))
    {
        std::cout << "[Registry] Folder does not exist.\n";
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(folderPath))
    {
        if (!entry.is_regular_file())
            continue;

        std::string ext = entry.path().extension().string();

        if (ext == ".h" || ext == ".hpp")
        {
            ParseFile(entry.path().string());
        }

    }

    std::cout << "[Registry] Total Registered Units: "
        << m_units.size() << "\n";
}

const std::vector<GV_Logic_Unit>& LogicUnitRegistry::GetAll() const
{
    return m_units;
}

