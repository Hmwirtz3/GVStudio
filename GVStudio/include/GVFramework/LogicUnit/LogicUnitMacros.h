#pragma once

#include <string>
#include <vector>
#include "GVFramework/Chunk/Chunk.h"

enum class ParamType
{
    Float,
    Int,
    Bool,
    String,
    Asset,
    Event,
    Message,
    Separator
};


#define _LU_ADD_PARAM(ptype, name, defval, mut, hintText) \
    { ParamType::ptype, #name, defval, mut, hintText }

#define UI_PARAM_FLOAT(name, defaultVal, hint) \
    _LU_ADD_PARAM(Float, name, std::to_string(defaultVal).c_str(), true, hint),

#define UI_PARAM_INT(name, defaultVal, hint) \
    _LU_ADD_PARAM(Int, name, std::to_string(defaultVal).c_str(), true, hint),

#define UI_PARAM_BOOL(name, defaultVal, hint) \
    _LU_ADD_PARAM(Bool, name, (defaultVal ? "true" : "false"), true, hint),

#define UI_PARAM_STRING(name, defaultVal, hint) \
    _LU_ADD_PARAM(String, name, defaultVal, true, hint),

#define UI_PARAM_ASSET(name, defaultVal, hint) \
    _LU_ADD_PARAM(Asset, name, defaultVal, true, hint),

#define UI_EVENT(name, defaultVal, hint) \
    _LU_ADD_PARAM(Event, name, defaultVal, true, hint),

#define UI_MESSAGE(name, defaultVal, hint) \
    _LU_ADD_PARAM(Message, name, defaultVal, true, hint),

#define UI_SEPARATOR(label) \
    _LU_ADD_PARAM(Separator, label, "", false, ""),

#define BEGIN_LOGIC_UNIT(unitName, chunk) \
    static LogicUnitDefinition unitName = { \
        #unitName, \
        chunk, \
        {

#define END_LOGIC_UNIT \
        } \
    };
