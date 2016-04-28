#include "ImGuiBase.h"

#include <panda/object/Dockable.h>
#include <panda/object/ObjectFactory.h>

#include <panda/types/DataTraits.h>
#include <panda/types/DataTypeId.h>

#include "imgui/imgui.h"

namespace panda 
{

template <class T>
class ImGui_Visualize : public DockableObject, public BaseImGuiObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(ImGui_Visualize, T), DockableObject)

	ImGui_Visualize(PandaDocument* doc)
		: DockableObject(doc)
		, m_fieldNameData(initData(getAutoFieldName(), "name", "The name of the field"))
		, m_value(initData("value", "The value to visualize using ImGui"))
	{
		addInput(m_fieldNameData);
		addInput(m_value);
	}

	std::string& fieldName() override
	{
		m_fieldName = m_fieldNameData.getValue();
		return m_fieldName;
	}

protected:
	std::string getAutoFieldName()
	{
		int type = panda::types::DataTypeId::getIdOf<T>();
		auto trait = panda::types::DataTraitsList::getTrait(type);
		return trait->typeDescription();
	}

	Data<std::string> m_fieldNameData;
	Data<T> m_value;
	std::string m_fieldName;
};

//****************************************************************************//

class ImGui_Visualize_Text : public ImGui_Visualize<std::string>
{
public:
	PANDA_CLASS(ImGui_Visualize_Text, PANDA_TEMPLATE(ImGui_Visualize, std::string))

	ImGui_Visualize_Text(PandaDocument* doc)
		: ImGui_Visualize<std::string>(doc)
	{
	}

	void fillGui() override
	{
		std::string value = m_value.getValue();
		ImGui::LabelText(m_fieldName.c_str(), value.c_str());
	}
};

int ImGui_Visualize_TextClass = RegisterObject<ImGui_Visualize_Text>("ImGui/Color/ImGui visualize text").setDescription("Create an ImGui field for visualizing a single text value");


} // namespace Panda
