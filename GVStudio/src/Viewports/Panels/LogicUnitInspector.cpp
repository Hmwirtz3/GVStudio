#include "Viewports/Panels/LogicUnitInspector.h"
#include "GVFramework/LogicUnit/LogicUnit.h"

#include "imgui/imgui.h"

void LogicUnitInspectorPanel::SetVisible(bool visible)
{
	m_isVisible = visible;
}

bool LogicUnitInspectorPanel::IsVisible() const
{
	return m_isVisible;
}

void LogicUnitInspectorPanel::Draw(SceneObject* selectedObject)
{
	if (!m_isVisible)
		return;

	ImGui::Begin("Logic Unit Inspector", &m_isVisible);

		if (!selectedObject)
		{
			ImGui::Text("No Object Selected.");
			ImGui::End();
			return;

		}

		if (!selectedObject->def || !selectedObject->def->def) //bad names!!
		{
			ImGui::Text("No Logic Unit Attached");
			ImGui::End();
			return;

		}

		GV_Logic_Unit_Instance& inst = *selectedObject->def;
		GV_Logic_Unit& def = *inst.def;

		ImGui::Text("Type: %s", def.typeName.c_str());
		ImGui::Separator();

        for (size_t i = 0; i < def.params.size(); i++)
        {
            const LU_Param_Def& paramDef = def.params[i];
            LU_Param_Val& value = inst.values[i];

            switch (paramDef.type)
            {
            case ParamType::Separator:
                ImGui::SeparatorText(paramDef.defaultValue.c_str());
                break;

            case ParamType::Float:
                ImGui::DragFloat(paramDef.name.c_str(), &value.fval, 0.1f);
                break;

            case ParamType::Int:
                ImGui::DragInt(paramDef.name.c_str(), &value.ival, 1);
                break;

            case ParamType::Bool:
                ImGui::Checkbox(paramDef.name.c_str(), &value.bval);
                break;

            case ParamType::String:
            case ParamType::Event:
            case ParamType::Message:
            {
                char buffer[256];
                buffer[0] = 0;
                strncpy_s(buffer, value.sval.c_str(), sizeof(buffer) - 1);

                if (ImGui::InputText(paramDef.name.c_str(), buffer, sizeof(buffer)))
                {
                    value.sval = buffer;
                }
                break;
            }

            default:
                break;
            }

            if (!paramDef.hint.empty() && ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", paramDef.hint.c_str());
        }

        ImGui::End();
}




