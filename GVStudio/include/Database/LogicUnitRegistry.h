#pragma once

#include <vector>

#include <string>
#include <unordered_map>

#include "GVFramework/LogicUnit/LogicUnit.h"

class LogicUnitRegistry
{
public:
	void Clear();

	void Register(const GV_Logic_Unit& unit);
	void Register(GV_Logic_Unit&& unit);

	const GV_Logic_Unit* Find(const std::string& typeName) const;

	
private: 
	std::vector<GV_Logic_Unit> m_units;
	std::unordered_map<std::string, size_t> m_lookup;
};