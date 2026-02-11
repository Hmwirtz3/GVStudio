#include "Database/LogicUnitRegistry.h"

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

}

void LogicUnitRegistry::Register(GV_Logic_Unit&& unit)
{
	size_t index = m_units.size();
	m_units.push_back(std::move(unit));
	m_lookup[m_units.back().typeName] = index;

}

const GV_Logic_Unit* LogicUnitRegistry::Find(const std::string& typeName) const
{
	auto it = m_lookup.find(typeName);
	if (it == m_lookup.end())
		return nullptr;

	return &m_units[it->second];
}

