#include "MiniXml/ObjectXml.h"
#include "GVFramework/Scene/SceneObject.h"
#include "MiniXML/MiniXml.h"

#include <cstdlib>
#include <iostream>


namespace
{
    const XmlAttribute* FindAttr(const XmlNode& node, const std::string& name)
    {
        for (const XmlAttribute& a : node.attributes)
            if (a.name == name)
                return &a;
        return nullptr;
    }

    const GV_Logic_Unit* FindDefinition(
        const std::vector<GV_Logic_Unit>& defs,
        const std::string& typeName)
    {
        for (const GV_Logic_Unit& d : defs)
            if (d.typeName == typeName)
                return &d;
        return nullptr;
    }

    int FindParamIndex(const GV_Logic_Unit& def, const std::string& paramName)
    {
        for (size_t i = 0; i < def.params.size(); ++i)
            if (def.params[i].name == paramName)
                return static_cast<int>(i);
        return -1;
    }
}

namespace ObjectXml
{
    bool LoadObjectFromXml(
        SceneObject& obj,
        const std::string& xmlPath,
        const std::vector<GV_Logic_Unit>& definitions)
    {
        XmlNode root;
        if (!XmlLoadFromFile(xmlPath, root))
            return false;

        if (root.name != "Object")
            return false;

        if (const XmlAttribute* n = FindAttr(root, "name"))
            if (obj.name.empty())
                obj.name = n->value;

        obj.def.reset();

        const XmlNode* luNode = nullptr;
        for (const XmlNode& child : root.children)
        {
            if (child.name == "LogicUnit")
            {
                luNode = &child;
                break;
            }
        }

        if (!luNode)
            return true;

        const XmlAttribute* nameAttr = FindAttr(*luNode, "name");
        if (!nameAttr)
            return true;

        const GV_Logic_Unit* def = FindDefinition(definitions, nameAttr->value);
        if (!def)
            return true;

        auto inst = std::make_unique<GV_Logic_Unit_Instance>();
        inst->def = const_cast<GV_Logic_Unit*>(def);
        inst->instanceName.clear();
        inst->values.resize(def->params.size());

        for (const XmlNode& pNode : luNode->children)
        {
            if (pNode.name != "Param")
                continue;

            const XmlAttribute* pn = FindAttr(pNode, "name");
            const XmlAttribute* pv = FindAttr(pNode, "value");
            if (!pn || !pv)
                continue;

            int idx = FindParamIndex(*def, pn->value);
            if (idx < 0)
                continue;

            const LU_Param_Def& pDef = def->params[idx];
            LU_Param_Val& pVal = inst->values[idx];

            const std::string& val = pv->value;

            switch (pDef.type)
            {
            case ParamType::Float:
                pVal.fval = std::strtof(val.c_str(), nullptr);
                break;

            case ParamType::Int:
                pVal.ival = std::strtol(val.c_str(), nullptr, 10);
                break;

            case ParamType::Bool:
                pVal.bval = (val == "true" || val == "1");
                break;

            case ParamType::String:
            case ParamType::Event:
            case ParamType::Message:
                pVal.sval = val;
                break;

            case ParamType::Separator:
                break;
            }
        }

        obj.def = std::move(inst);
        return true;
    }

    bool SaveObjectToXml(
        const SceneObject& obj,
        const std::string& xmlPath,
        const std::vector<GV_Logic_Unit>&)
    {
        XmlNode root;
        root.name = "Object";
        root.attributes.push_back({ "name", obj.name });

        if (obj.def && obj.def->def)
        {
            const GV_Logic_Unit_Instance& inst = *obj.def;
            const GV_Logic_Unit& def = *inst.def;

            XmlNode luNode;
            luNode.name = "LogicUnit";
            luNode.attributes.push_back({ "name", def.typeName });

            size_t count = std::min(def.params.size(), inst.values.size());

            for (size_t i = 0; i < count; ++i)
            {
                const LU_Param_Def& pDef = def.params[i];
                const LU_Param_Val& pVal = inst.values[i];

                if (pDef.type == ParamType::Separator)
                    continue;

                XmlNode pNode;
                pNode.name = "Param";
                pNode.attributes.push_back({ "name", pDef.name });

                XmlAttribute v;
                v.name = "value";

                switch (pDef.type)
                {
                case ParamType::Float:
                    v.value = std::to_string(pVal.fval);
                    break;

                case ParamType::Int:
                    v.value = std::to_string(pVal.ival);
                    break;

                case ParamType::Bool:
                    v.value = pVal.bval ? "true" : "false";
                    break;

                case ParamType::String:
                case ParamType::Event:
                case ParamType::Message:
                    v.value = pVal.sval;
                    break;

                default:
                    break;
                }

                pNode.attributes.push_back(v);
                luNode.children.push_back(std::move(pNode));
            }

            root.children.push_back(std::move(luNode));
        }

        return XmlSaveToFile(xmlPath, root);
    }
}
