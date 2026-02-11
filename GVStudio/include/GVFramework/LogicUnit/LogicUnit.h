#pragma once

#include "GVFramework/Chunk/Chunk.h"

#include <vector>
#include <string>

enum class ParamType
{
	Float,
	Int,
	Bool,
	String,
	Separator,
	Event,
	Message
};

struct LU_Param_Def // Params gotten from parser
{
	std::string name;
	ParamType type;
	bool isMutable = false;
	std::string defaultValue;
	std::string hint;
};

struct LU_Param_Val // Parameter values used by editor
{
	
	float fval = 0.0f;
	int ival = 0;
	bool bval = false;
	std::string sval;

	char stringBuffer[256] = {};
};

struct GV_Logic_Unit // Logic unit gotten from parser
{
	std::string typeName;
	GV_ChunkType chunkType;
	std::vector<LU_Param_Def> params;
};

struct GV_Logic_Unit_Instance // instance of logic unit, used by editor
{
	GV_Logic_Unit* def;
	std::string instanceName;
	std::vector<LU_Param_Val> values;
};