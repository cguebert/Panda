#include <ui/graphview/DataLabelAddon.h>
#include <panda/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>

#include <set>

DataLabelAddon::DataLabelAddon(panda::PandaObject& object) 
	: panda::BaseObjectAddon(object) 
{ 
	m_observer.get(object.parentDocument()->getSignals().modifiedObject).connect<DataLabelAddon, &DataLabelAddon::modifiedObject>(this);
}

void DataLabelAddon::setDefinition(panda::ObjectAddonNodeDefinition& nodeDefinition)
{
	auto& labelNode = nodeDefinition.addChild("DataLabel", true);
	labelNode.addAttribute("data");
}

void DataLabelAddon::save(panda::ObjectAddonNode& node)
{ 
	for (const auto& dl : m_dataLabels)
	{
		auto dlNode = node.addChild("DataLabel");
		dlNode.setAttribute("data", dl.data->getName());
		dlNode.setText(dl.label);
	}
}

void DataLabelAddon::load(const panda::ObjectAddonNode& node)
{ 
	auto e = node.firstChild("DataLabel");
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

void DataLabelAddon::modifiedObject(panda::PandaObject* object)
{
	if (object == &m_object)
	{
		std::set<panda::BaseData*> datas;
		for (auto data : object->getDatas())
			datas.insert(data);
		auto last = std::remove_if(m_dataLabels.begin(), m_dataLabels.end(), [&datas](const auto& dl) {
			return !datas.count(dl.data);
		});
		m_dataLabels.erase(last, m_dataLabels.end());
	}
}

//****************************************************************************//

std::string getDataLabel(panda::BaseData* data)
{
	auto addon = data->getOwner()->addons().get<DataLabelAddon>();
	if(addon)
		return addon->getLabel(data);
	return {};
}

void setDataLabel(panda::BaseData* data, const std::string& label)
{
	data->getOwner()->addons().edit<DataLabelAddon>().setLabel(data, label);
}

//****************************************************************************//

int DataLabelAddon_Reg = panda::RegisterObjectAddon<DataLabelAddon>();