#include <ui/DataWidgetFactory.h>

#include <panda/BaseData.h>
#include <panda/DataFactory.h>

DataWidgetFactory* DataWidgetFactory::getInstance()
{
	static DataWidgetFactory instance;
	return &instance;
}

BaseDataWidget* DataWidgetFactory::create(panda::BaseData* data, QWidget* parent)
{
	int fullType = data->getDataTrait()->fullTypeId();
	const QMap<QString, DataWidgetEntryPtr>& map = registry.value(fullType);
	if(map.isEmpty())
		return nullptr;

	// Special case : for lists and animations, we use the same DataWidget which will create other ones later
	if(map.size() == 1)
		return map.begin().value()->creator->create(data, parent);

	QString widgetName = data->getWidget();
	DataWidgetEntry* entry = map.value(widgetName).data();
	if(!entry)	// If the custom widget doesn't exist, use the default one
		entry = map.value("default").data();

	// If a default one doesn't exist, we don't know which one to use
	if(entry)
		return entry->creator->create(data, parent);

	return nullptr;
}

DataWidgetFactory::DataWidgetEntry* DataWidgetFactory::getEntry(int fullType, QString widgetName)
{
	return registry.value(fullType).value(widgetName).data();
}

QList<QString> DataWidgetFactory::getWidgetNames(int fullType)
{
	return registry.value(fullType).keys();
}

void DataWidgetFactory::registerWidget(int fullType, QString widgetName, DataWidgetCreatorPtr creator)
{
	DataWidgetEntryPtr entry = DataWidgetEntryPtr::create();
	entry->fullType = fullType;
	entry->widgetName = widgetName;
	entry->creator = creator;

	registry[fullType][widgetName] = entry;
}
