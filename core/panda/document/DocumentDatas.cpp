#include <panda/document/DocumentDatas.h>
#include <panda/data/DataFactory.h>
#include <panda/object/PandaObject.h>
#include <panda/XmlDocument.h>

#include <panda/helper/algorithm.h>

namespace panda 
{

DocumentDatas::DocumentDatas(PandaObject* parent)
	: m_parent(parent)
{
}

void DocumentDatas::set(DataList list)
{
	m_dataList = std::move(list);
}

void DocumentDatas::add(const DataPtr& data, int index)
{ 
	if(index < 0 || index >= static_cast<int>(m_dataList.size()))
		m_dataList.push_back(data); 
	else
		m_dataList.insert(m_dataList.begin() + index, data);
}

void DocumentDatas::remove(const DataPtr& data)
{
	auto it = std::find(m_dataList.begin(), m_dataList.end(), data);
	if (it != m_dataList.end())
		m_dataList.erase(it);
}

void DocumentDatas::reorder(const DataPtr& data, int index)
{
	auto it = std::find(m_dataList.begin(), m_dataList.end(), data);
	if (it != m_dataList.end())
		helper::slide(it, it + 1, m_dataList.begin() + index);
	else
		m_dataList.insert(m_dataList.begin() + index, data);
}

void DocumentDatas::save(XmlElement& elem)
{
	for(const auto& data : m_dataList)
	{
		auto node = elem.addChild("GroupData");
		node.setAttribute("type", DataFactory::typeToName(data->getDataTrait()->fullTypeId()));
		node.setAttribute("input", data->isInput());
		node.setAttribute("output", data->isOutput());
		node.setAttribute("name", data->getName());
		node.setAttribute("help", data->getHelp());

		const auto widget = data->getWidget();
		const auto widgetData = data->getWidgetData();
		if(!widget.empty())
			node.setAttribute("widget", widget);
		if(!widgetData.empty())
			node.setAttribute("widgetData", widgetData);
	}
}

void DocumentDatas::load(const XmlElement& elem)
{
	for (auto groupDataNode = elem.firstChild("GroupData"); groupDataNode; groupDataNode = groupDataNode.nextSibling("GroupData"))
	{
		uint32_t type, input, output;
		std::string name, help, widget, widgetData;
		type = DataFactory::nameToType(groupDataNode.attribute("type").toString());
		input = groupDataNode.attribute("input").toUnsigned();
		output = groupDataNode.attribute("output").toUnsigned();
		name = groupDataNode.attribute("name").toString();
		help = groupDataNode.attribute("help").toString();
		widget = groupDataNode.attribute("widget").toString();
		widgetData = groupDataNode.attribute("widgetData").toString();

		auto dataPtr = DataFactory::getInstance()->create(type, name, help, m_parent);
		auto data = dataPtr.get();
		if (!widget.empty())
			data->setWidget(widget);
		if (!widgetData.empty())
			data->setWidgetData(widgetData);
		add(dataPtr);
		if (input)
			m_parent->addInput(*data);
		if (output)
			m_parent->addOutput(*data);
	}
}

} // namespace panda


