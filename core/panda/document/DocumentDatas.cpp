#include <panda/document/DocumentDatas.h>
#include <panda/data/DataFactory.h>
#include <panda/object/PandaObject.h>
#include <panda/XmlDocument.h>

#include <panda/helper/algorithm.h>

namespace panda 
{

std::string findAvailableDataName(PandaObject* parent, const std::string& baseName, BaseData* data)
{
	auto name = baseName;
	BaseData* testData = parent->getData(name);
	if(testData && testData != data)
	{
		int i=2;
		testData = parent->getData(name + std::to_string(i));
		while(testData && testData != data)
		{
			++i;
			testData = parent->getData(name + std::to_string(i));
		}
		name = name + std::to_string(i);
	}
	return name;
}

std::shared_ptr<BaseData> duplicateData(PandaObject* parent, BaseData* data)
{
	auto name = findAvailableDataName(parent, data->getName());

	auto newData = DataFactory::create(data->getDataTrait()->fullTypeId(),
									   name, data->getHelp(), parent);
	newData->setDisplayed(data->isDisplayed());
	newData->setPersistent(data->isPersistent());
	newData->setWidget(data->getWidget());
	newData->setWidgetData(data->getWidgetData());
	newData->setInput(data->isInput());
	newData->setOutput(data->isOutput());

	return newData;
}

//****************************************************************************//

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

void DocumentDatas::save(XmlElement& elem, const std::string& nodeName)
{
	for(const auto& data : m_dataList)
	{
		auto node = elem.addChild(nodeName);
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

void DocumentDatas::load(const XmlElement& elem, const std::string& nodeName)
{
	for (auto node = elem.firstChild(nodeName); node; node = node.nextSibling(nodeName))
	{
		uint32_t type, input, output;
		std::string name, help, widget, widgetData;
		type = DataFactory::nameToType(node.attribute("type").toString());
		input = node.attribute("input").toUnsigned();
		output = node.attribute("output").toUnsigned();
		name = node.attribute("name").toString();
		help = node.attribute("help").toString();
		widget = node.attribute("widget").toString();
		widgetData = node.attribute("widgetData").toString();

		auto dataPtr = DataFactory::create(type, name, help, m_parent);
		if (!dataPtr)
			continue;

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


