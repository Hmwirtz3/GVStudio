#include "GVFramework/LogicUnit/LogicUnitParser.h"
#include "GVFramework/Chunk/Chunk.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

static GV_ChunkType ParseChunkType(const std::string& s)
{
    if (s == "GV_CHUNK_TEXTURE")
        return GV_ChunkType::GV_CHUNK_TEXTURE;

    if (s == "GV_CHUNK_HEIGHTMAP")
        return GV_ChunkType::GV_CHUNK_HEIGHTMAP;

    if (s == "GV_CHUNK_STATIC_MESH")
        return GV_ChunkType::GV_CHUNK_STATIC_MESH;

    return GV_ChunkType::GV_CHUNK_UNKNOWN;
}

std::string LogicUnitParser::Trim(const std::string& str)
{
    const size_t first = str.find_first_not_of(" \t\r\n");
    const size_t last = str.find_last_not_of(" \t\r\n");

    if (first == std::string::npos)
        return "";

    return str.substr(first, last - first + 1);
}

std::string LogicUnitParser::StripQuotes(const std::string& s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);

    return s;
}

std::vector<GV_Logic_Unit> LogicUnitParser::ParseFile(const std::string& filename)
{
    std::vector<GV_Logic_Unit> units;

    std::cout << "\n[Parser] Parsing file: " << filename << "\n";

    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "[Parser] Cannot open file: " << filename << "\n";
        return units;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    size_t pos = 0;

    while ((pos = content.find("BEGIN_LOGIC_UNIT(", pos)) != std::string::npos)
    {
        const size_t nameStart = pos + strlen("BEGIN_LOGIC_UNIT(");
        const size_t nameEnd = content.find(")", nameStart);
        if (nameEnd == std::string::npos)
            break;

        std::string args = Trim(content.substr(nameStart, nameEnd - nameStart));

        size_t comma = args.find(',');
        if (comma == std::string::npos)
            break;

        std::string unitName = Trim(args.substr(0, comma));
        std::string chunkStr = Trim(args.substr(comma + 1));

        GV_Logic_Unit unit;
        unit.typeName = unitName;
        unit.chunkType = ParseChunkType(chunkStr);

        std::cout << "--------------------------------------\n";
        std::cout << "[Parser] Found Logic Unit: " << unit.typeName << "\n";
        std::cout << "[Parser] Chunk Type: " << chunkStr << "\n";

        const size_t bodyStart = nameEnd + 1;
        const size_t bodyEnd = content.find("END_LOGIC_UNIT", bodyStart);
        if (bodyEnd == std::string::npos)
            break;

        std::string block = content.substr(bodyStart, bodyEnd - bodyStart);

        size_t macroPos = 0;

        while ((macroPos = block.find("UI_", macroPos)) != std::string::npos)
        {
            const size_t end = block.find(")", macroPos);
            if (end == std::string::npos)
                break;

            std::string line = Trim(block.substr(macroPos, end - macroPos + 1));

            LU_Param_Def def{};
            def.isMutable = true;

            if (line.find("UI_SEPARATOR") != std::string::npos)
            {
                size_t q1 = line.find('"');
                size_t q2 = line.rfind('"');

                def.type = ParamType::Separator;
                def.name = "__SEPARATOR__";
                def.defaultValue =
                    (q1 != std::string::npos && q2 != std::string::npos && q2 > q1)
                    ? line.substr(q1 + 1, q2 - q1 - 1)
                    : "";
                def.hint = "";

                std::cout << "  [Param] Separator: " << def.defaultValue << "\n";

                unit.params.push_back(def);
                macroPos = end + 1;
                continue;
            }

            const size_t lparen = line.find("(");
            const size_t rparen = line.find(")", lparen);

            if (lparen == std::string::npos || rparen == std::string::npos)
            {
                macroPos = end + 1;
                continue;
            }

            std::string argsString = line.substr(lparen + 1, rparen - lparen - 1);

            std::vector<std::string> args;
            std::stringstream ss(argsString);
            std::string token;

            while (std::getline(ss, token, ','))
                args.push_back(Trim(token));

            if (args.size() < 3)
            {
                macroPos = end + 1;
                continue;
            }

            def.name = args[0];
            def.defaultValue = StripQuotes(args[1]);
            def.hint = StripQuotes(args[2]);

            if (line.find("UI_PARAM_FLOAT") != std::string::npos)
                def.type = ParamType::Float;
            else if (line.find("UI_PARAM_INT") != std::string::npos)
                def.type = ParamType::Int;
            else if (line.find("UI_PARAM_BOOL") != std::string::npos)
                def.type = ParamType::Bool;
            else if (line.find("UI_PARAM_STRING") != std::string::npos)
                def.type = ParamType::String;
            else if (line.find("UI_PARAM_ASSET") != std::string::npos)
                def.type = ParamType::String;
            else if (line.find("UI_EVENT") != std::string::npos)
                def.type = ParamType::Event;
            else if (line.find("UI_MESSAGE") != std::string::npos)
                def.type = ParamType::Message;
            else
            {
                macroPos = end + 1;
                continue;
            }

            std::cout << "  [Param] Name: " << def.name
                << " | Type: " << static_cast<int>(def.type)
                << " | Default: " << def.defaultValue
                << " | Hint: " << def.hint << "\n";

            unit.params.push_back(def);
            macroPos = end + 1;
        }

        std::cout << "[Parser] Total Params: " << unit.params.size() << "\n";

        units.push_back(unit);
        pos = bodyEnd + strlen("END_LOGIC_UNIT");
    }

    std::cout << "[Parser] Units Parsed From File: " << units.size() << "\n";

    return units;
}
