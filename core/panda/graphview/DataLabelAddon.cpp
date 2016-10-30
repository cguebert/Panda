#include <panda/graphview/DataLabelAddon.h>
#include <panda/document/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>

#include <set>

namespace panda
{

namespace graphview
{

DataLabelAddon::DataLabelAddon(PandaObject& object) 
	: BaseObjectAddon(object) 
{
}

void DataLabelAddon::setDefinition(ObjectAddonNodeDefinition& nodeDefinition)
{
	auto& labelNode = nodeDefinition.addChild("DataLabel", true);
	labelNode.addAttribute("data");
}

void DataLabelAddon::save(ObjectAddonNode& node)
{ 
	for (const auto& dl : m_dataLabels)
	{
		auto dlNode = node.addChild("DataLabel");
		dlNode.setAttribute("data", dl.data->getName());
		dlNode.setText(dl.label);
	}
}

void DataLabelAddon::load(const ObjectAddonNode& node)
{ 
	for(auto e = node.firstChild("DataLabel"); e; e = e.nextSibling("DataLabel"))
	{
		auto data = m_object.getData(e.attribute("data").toString());
		if (data)
		{
			DataLabel dl;
			dl.data = data;
			dl.label = e.text();
			m_dataLabels.push_back(dl);
		}
	}
}

std::string DataLabelAddon::getLabel(BaseData* data) const
{
	auto it = std::find_if(m_dataLabels.begin(), m_dataLabels.end(), [data](const auto& dl) {
		return dl.data == data;
	});
	if (it != m_dataLabels.end())
		return it->label;
	return "";
}

void DataLabelAddon::setLabel(BaseData* data, const std::string& label)
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

void DataLabelAddon::objectModified()
{
	std::set<BaseData*> datas;
	for (auto data : m_object.getDatas())
		datas.insert(data);
	auto last = std::remove_if(m_dataLabels.begin(), m_dataLabels.end(), [&datas](const auto& dl) {
		return !datas.count(dl.data);
	});
	m_dataLabels.erase(last, m_dataLabels.end());
}

//****************************************************************************//

std::string DataLabelAddon::getDataLabel(BaseData* data)
{
	auto addon = data->getOwner()->addons().get<DataLabelAddon>();
	if(addon)
		return addon->getLabel(data);
	return {};
}

void DataLabelAddon::setDataLabel(BaseData* data, const std::string& label)
{
	data->getOwner()->addons().edit<DataLabelAddon>().setLabel(data, label);
}

//****************************************************************************//

int DataLabelAddon_Reg = RegisterObjectAddon<DataLabelAddon>();

} // namespace graphview

} // namespace panda
