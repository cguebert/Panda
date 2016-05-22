#include <ui/graphview/DataLabelAddon.h>
#include <panda/XmlDocument.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>

DataLabelAddon::DataLabelAddon(panda::PandaObject& object) 
	: panda::BaseObjectAddon(object) 
{ }

void DataLabelAddon::save(panda::XmlElement& elem)
{ 
	for (const auto& dl : m_dataLabels)
	{
		auto dlNode = elem.addChild("DataLabel");
		dlNode.setAttribute("data", dl.data->getName());
		dlNode.setText(dl.label);
	}
}

void DataLabelAddon::load(panda::XmlElement& elem)
{ 
	auto e = elem.firstChild("DataLabel");
	while(e)
	{
		auto data = m_object.getData(e.attribute("data").toString());
		if (data)
		{
			DataLabel dl;
			dl.data = data;
			dl.label = e.text();
			m_dataLabels.push_back(dl);
		}
		e = e.nextSibling("DataLabel");
	}
}

std::string DataLabelAddon::getLabel(panda::BaseData* data) const
{
	auto it = std::find_if(m_dataLabels.begin(), m_dataLabels.end(), [data](const auto& dl) {
		return dl.data == data;
	});
	if (it != m_dataLabels.end())
		return it->label;
	return "";
}

void DataLabelAddon::setLabel(panda::BaseData* data, const std::string& label)
{
	auto it = std::find_if(m_dataLabels.begin(), m_dataLabels.end(), [data](const auto& dl) {
		return dl.data == data;
	});
	if (label.empty()) // Remove the label
	{
		if (it != m_dataLabels.end())
			m_dataLabels.erase(it);
	}
	else // Modify or add the labl=el
	{
		if (it != m_dataLabels.end())
			it->label = label;
		else
		{
			DataLabel dl;
			dl.data = data;
			dl.label = label;
			m_dataLabels.push_back(std::move(dl));
		}
	}
}

//****************************************************************************//

std::string getDataLabel(panda::BaseData* data)
{
	return data->getOwner()->addons().get<DataLabelAddon>().getLabel(data);
}

void setDataLabel(panda::BaseData* data, const std::string& label)
{
	data->getOwner()->addons().get<DataLabelAddon>().setLabel(data, label);
}

//****************************************************************************//

int DataLabelAddon_Reg = panda::RegisterObjectAddon<DataLabelAddon>();