#include <ui/widget/DataWidgetFactory.h>

#include <panda/data/DataFactory.h>
#include <panda/helper/algorithm.h>

DataWidgetFactory* DataWidgetFactory::getInstance()
{
	static DataWidgetFactory instance;
	return &instance;
}

BaseDataWidget* DataWidgetFactory::create(QWidget* parent, panda::BaseData* data) const
{
	const int fullType = data->getDataTrait()->fullTypeId();
	const QString widgetName = QString::fromStdString(data->getWidget());

	const BaseDataWidgetCreator* creator = getCreator(fullType, widgetName);
	if(creator)
		return creator->create(parent, data);
	return nullptr;
}

BaseDataWidget* DataWidgetFactory::create(QWidget* parent, void* pValue, int fullType,
										  QString widget, QString displayName, QString parameters) const
{
	const BaseDataWidgetCreator* creator = getCreator(fullType, widget);
	if(creator)
		return creator->create(parent, pValue, widget, displayName, parameters);
	return nullptr;
}

const BaseDataWidgetCreator* DataWidgetFactory::getCreator(int fullType, QString widgetName) const
{
	if (!registry.count(fullType))
		return nullptr;
	const auto& map = registry.at(fullType);

	// Special case : for lists and animations, we use the same DataWidget which will create other ones later
	if(map.size() == 1)
		return map.begin()->second->creator.get();

	auto entry = panda::helper::valueOrDefault(map, widgetName);
	if(!entry)	// If the custom widget doesn't exist, first look for a generic one
		entry = panda::helper::valueOrDefault(map, "generic");

	if(!entry && map.count("default"))	// Then use the default one
		entry = panda::helper::valueOrDefault(map, "default");

	// If a default one doesn't exist, we don't know which one to use
	if(entry)
		return entry->creator.get();

	return nullptr;
}

const DataWidgetFactory::DataWidgetEntry* DataWidgetFactory::getEntry(int fullType, QString widgetName) const
{
	auto it1 = registry.find(fullType);
	if (it1 == registry.end())
		return nullptr;

	const auto& widgetMap = it1->second;
	auto it2 = widgetMap.find(widgetName);
	if (it2 == widgetMap.end())
		return nullptr;

	return it2->second.get();
}

std::vector<QString> DataWidgetFactory::getWidgetNames(int fullType) const
{
	std::vector<QString> result;
	const auto& widgetMap = panda::helper::valueOrDefault(registry, fullType);
	for (const auto& w : widgetMap)
		result.push_back(w.first);
	return result;
}

void DataWidgetFactory::registerWidget(int fullType, QString widgetName, DataWidgetCreatorPtr creator)
{
	DataWidgetEntryPtr entry = std::make_shared<DataWidgetEntry>();
	entry->fullType = fullType;
	entry->widgetName = widgetName;
	entry->creator = creator;

	registry[fullType][widgetName] = entry;
}
