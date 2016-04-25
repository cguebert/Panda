#include "ImGuiBase.h"

#include <panda/object/Dockable.h>
#include <panda/object/ObjectFactory.h>

#include <panda/types/DataTraits.h>
#include <panda/types/DataTypeId.h>
#include <panda/types/Color.h>
#include <panda/types/Rect.h>

#include "imgui/imgui.h"

namespace panda 
{

template <class T>
class ImGui_Data : public DockableObject, public BaseImGuiObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(ImGui_Data, T), DockableObject)

	ImGui_Data(PandaDocument* doc)
		: DockableObject(doc)
		, m_fieldNameData(initData(getAutoFieldName(), "name", "The name of the field"))
		, m_initValue(initData("init", "The initial value"))
		, m_outputValue(initData("value", "The value, potentially modified using ImGui"))
	{
		addInput(m_fieldNameData);
		addInput(m_initValue);
		
		addOutput(m_outputValue);
	}

	void reset()
	{
		m_outputValue.setValue(m_initValue.getValue());
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
	Data<T> m_initValue, m_outputValue;
	std::string m_fieldName;
};

//****************************************************************************//

class ImGui_Data_IntSimple : public ImGui_Data<int>
{
public:
	PANDA_CLASS(ImGui_Data_IntSimple, PANDA_TEMPLATE(ImGui_Data, int))

		ImGui_Data_IntSimple(PandaDocument* doc)
		: ImGui_Data<int>(doc)
		, m_step(initData(0, "step", "Step for modifying the value using the +/- buttons"))
	{
		addInput(m_step);
	}

	void fillGui() override
	{
		int step = m_step.getValue();
		int value = m_outputValue.getValue();
		if (ImGui::InputInt(m_fieldName.c_str(), &value, step, step * 10))
			m_outputValue.setValue(value);
	}

	Data<int> m_step;
};

int ImGui_Data_IntSimpleClass = RegisterObject<ImGui_Data_IntSimple>("ImGui/Integer/ImGui integer").setDescription("Create an ImGui field for editing a single integer value");

//****************************************************************************//

class ImGui_Data_IntSlider : public ImGui_Data<int>
{
public:
	PANDA_CLASS(ImGui_Data_IntSlider, PANDA_TEMPLATE(ImGui_Data, int))

		ImGui_Data_IntSlider(PandaDocument* doc)
		: ImGui_Data<int>(doc)
		, m_min(initData(0, "min", "Minimum value of the slider"))
		, m_max(initData(100, "max", "Maximum value of the slider"))
	{
		addInput(m_min);
		addInput(m_max);
	}

	void fillGui() override
	{
		int minV = m_min.getValue(), maxV = m_max.getValue();
		int value = m_outputValue.getValue();
		if (ImGui::SliderInt(m_fieldName.c_str(), &value, minV, maxV))
			m_outputValue.setValue(value);
	}

	Data<int> m_min, m_max;
};

int ImGui_Data_IntSliderClass = RegisterObject<ImGui_Data_IntSlider>("ImGui/Integer/ImGui integer slider").setDescription("Create an ImGui field for editing a single integer value using a slider");

//****************************************************************************//

class ImGui_Data_IntCheckbox : public ImGui_Data<int>
{
public:
	PANDA_CLASS(ImGui_Data_IntCheckbox, PANDA_TEMPLATE(ImGui_Data, int))

	ImGui_Data_IntCheckbox(PandaDocument* doc)
		: ImGui_Data<int>(doc) { }

	void fillGui() override
	{
		bool value = m_outputValue.getValue() != 0;
		if (ImGui::Checkbox(m_fieldName.c_str(), &value))
			m_outputValue.setValue(value ? 1 : 0);
	}
};

int ImGui_Data_IntCheckboxClass = RegisterObject<ImGui_Data_IntCheckbox>("ImGui/Integer/ImGui integer checkbox").setDescription("Create an ImGui field for editing a single integer value using a checkbox");


//****************************************************************************//

class ImGui_Data_FloatSimple : public ImGui_Data<float>
{
public:
	PANDA_CLASS(ImGui_Data_FloatSimple, PANDA_TEMPLATE(ImGui_Data, float))

	ImGui_Data_FloatSimple(PandaDocument* doc)
		: ImGui_Data<float>(doc)
		, m_step(initData(0, "step", "Step for modifying the value using the +/- buttons"))
	{
		addInput(m_step);
	}

	void fillGui() override
	{
		float step = m_step.getValue();
		float value = m_outputValue.getValue();
		if (ImGui::InputFloat(m_fieldName.c_str(), &value, step, step * 10))
			m_outputValue.setValue(value);
	}

	Data<float> m_step;
};

int ImGui_Data_FloatSimpleClass = RegisterObject<ImGui_Data_FloatSimple>("ImGui/Real/ImGui real").setDescription("Create an ImGui field for editing a single real value");

//****************************************************************************//

class ImGui_Data_FloatSlider : public ImGui_Data<float>
{
public:
	PANDA_CLASS(ImGui_Data_FloatSlider, PANDA_TEMPLATE(ImGui_Data, float))

		ImGui_Data_FloatSlider(PandaDocument* doc)
		: ImGui_Data<float>(doc)
		, m_min(initData(0, "min", "Minimum value of the slider"))
		, m_max(initData(1, "max", "Maximum value of the slider"))
	{
		addInput(m_min);
		addInput(m_max);
	}

	void fillGui() override
	{
		float minV = m_min.getValue(), maxV = m_max.getValue();
		float value = m_outputValue.getValue();
		if (ImGui::SliderFloat(m_fieldName.c_str(), &value, minV, maxV))
			m_outputValue.setValue(value);
	}

	Data<float> m_min, m_max;
};

int ImGui_Data_FloatSliderClass = RegisterObject<ImGui_Data_FloatSlider>("ImGui/Real/ImGui real slider").setDescription("Create an ImGui field for editing a single real value using a slider");

//****************************************************************************//

class ImGui_Data_PointSimple : public ImGui_Data<types::Point>
{
public:
	PANDA_CLASS(ImGui_Data_PointSimple, PANDA_TEMPLATE(ImGui_Data, types::Point))

		ImGui_Data_PointSimple(PandaDocument* doc)
		: ImGui_Data<types::Point>(doc) {}

	void fillGui() override
	{
		types::Point value = m_outputValue.getValue();
		if (ImGui::InputFloat2(m_fieldName.c_str(), value.data()))
			m_outputValue.setValue(value);
	}
};

int ImGui_Data_PointSimpleClass = RegisterObject<ImGui_Data_PointSimple>("ImGui/Point/ImGui point").setDescription("Create an ImGui field for editing a single point value");

//****************************************************************************//

class ImGui_Data_PointSlider : public ImGui_Data<types::Point>
{
public:
	PANDA_CLASS(ImGui_Data_PointSlider, PANDA_TEMPLATE(ImGui_Data, types::Point))

		ImGui_Data_PointSlider(PandaDocument* doc)
		: ImGui_Data<types::Point>(doc)
		, m_min(initData(0, "min", "Minimum value of the slider"))
		, m_max(initData(1, "max", "Maximum value of the slider"))
	{
		addInput(m_min);
		addInput(m_max);
	}

	void fillGui() override
	{
		float minV = m_min.getValue(), maxV = m_max.getValue();
		types::Point value = m_outputValue.getValue();
		if (ImGui::SliderFloat2(m_fieldName.c_str(), value.data(), minV, maxV))
			m_outputValue.setValue(value);
	}

	Data<float> m_min, m_max;
};

int ImGui_Data_PointSliderClass = RegisterObject<ImGui_Data_PointSlider>("ImGui/Point/ImGui point slider").setDescription("Create an ImGui field for editing a single point value using sliders");

//****************************************************************************//

class ImGui_Data_RectSimple : public ImGui_Data<types::Rect>
{
public:
	PANDA_CLASS(ImGui_Data_RectSimple, PANDA_TEMPLATE(ImGui_Data, types::Rect))

		ImGui_Data_RectSimple(PandaDocument* doc)
		: ImGui_Data<types::Rect>(doc) {}

	void fillGui() override
	{
		types::Rect value = m_outputValue.getValue();
		if (ImGui::InputFloat4(m_fieldName.c_str(), value.data()))
			m_outputValue.setValue(value);
	}
};

int ImGui_Data_RectSimpleClass = RegisterObject<ImGui_Data_RectSimple>("ImGui/Rectangle/ImGui rectangle").setDescription("Create an ImGui field for editing a single rectangle value");

//****************************************************************************//

class ImGui_Data_RectSlider : public ImGui_Data<types::Rect>
{
public:
	PANDA_CLASS(ImGui_Data_RectSlider, PANDA_TEMPLATE(ImGui_Data, types::Rect))

		ImGui_Data_RectSlider(PandaDocument* doc)
		: ImGui_Data<types::Rect>(doc)
		, m_min(initData(0, "min", "Minimum value of the slider"))
		, m_max(initData(1, "max", "Maximum value of the slider"))
	{
		addInput(m_min);
		addInput(m_max);
	}

	void fillGui() override
	{
		float minV = m_min.getValue(), maxV = m_max.getValue();
		types::Rect value = m_outputValue.getValue();
		if (ImGui::SliderFloat4(m_fieldName.c_str(), value.data(), minV, maxV))
			m_outputValue.setValue(value);
	}

	Data<float> m_min, m_max;
};

int ImGui_Data_RectSliderClass = RegisterObject<ImGui_Data_RectSlider>("ImGui/Rectangle/ImGui rectangle slider").setDescription("Create an ImGui field for editing a single rectangle value using sliders");

//****************************************************************************//

class ImGui_Data_ColorSimple : public ImGui_Data<types::Color>
{
public:
	PANDA_CLASS(ImGui_Data_ColorSimple, PANDA_TEMPLATE(ImGui_Data, types::Color))

		ImGui_Data_ColorSimple(PandaDocument* doc)
		: ImGui_Data<types::Color>(doc)
		, m_alpha(initData(0, "edit alpha", "If true, show the alpha to also be edited"))
	{
		addInput(m_alpha);
		m_alpha.setWidget("checkbox");
	}

	void fillGui() override
	{
		types::Color value = m_outputValue.getValue();
		bool editAlpha = m_alpha.getValue() != 0;
		if (ImGui::ColorEdit4(m_fieldName.c_str(), value.data(), editAlpha))
			m_outputValue.setValue(value);
	}

	Data<int> m_alpha;
};

int ImGui_Data_ColorSimpleClass = RegisterObject<ImGui_Data_ColorSimple>("ImGui/Color/ImGui color").setDescription("Create an ImGui field for editing a single color value");

//****************************************************************************//

class ImGui_Data_StringSimple : public ImGui_Data<std::string>
{
public:
	PANDA_CLASS(ImGui_Data_StringSimple, PANDA_TEMPLATE(ImGui_Data, std::string))

	ImGui_Data_StringSimple(PandaDocument* doc)
		: ImGui_Data<std::string>(doc)
		, m_maxLength(initData(255, "max length", "Maximum text length"))
	{
		addInput(m_maxLength);
	}

	void fillGui() override
	{
		std::string value = m_outputValue.getValue();
		int bufSize = m_maxLength.getValue();
		if (bufSize < 1)
			bufSize = 1;
		value.reserve(bufSize);
		if (ImGui::InputText(m_fieldName.c_str(), &value[0], bufSize-1))
			m_outputValue.setValue(value.c_str());
	}

	Data<int> m_maxLength;
};

int ImGui_Data_StringSimpleClass = RegisterObject<ImGui_Data_StringSimple>("ImGui/Color/ImGui text").setDescription("Create an ImGui field for editing a single text value");


} // namespace Panda
