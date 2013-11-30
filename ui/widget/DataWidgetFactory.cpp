#include <ui/widget/DataWidgetFactory.h>

#include <panda/BaseData.h>
#include <panda/DataFactory.h>

DataWidgetFactory* DataWidgetFactory::getInstance()
{
	static DataWidgetFactory instance;
	return &instance;
}

BaseDataWidget* DataWidgetFactory::create(QWidget* parent, panda::BaseData* data) const
{
	const int fullType = data->getDataTrait()->fullTypeId();
	const QString widgetName = data->getWidget();

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
	const QMap<QString, DataWidgetEntryPtr>& map = registry.value(fullType);
	if(map.isEmpty())
		return nullptr;

	// Special case : for lists and animations, we use the same DataWidget which will create other ones later
	if(map.size() == 1)
		return map.begin().value()->creator.data();

	DataWidgetEntry* entry = map.value(widgetName).data();
	if(!entry)	// If the custom widget doesn't exist, first look for a generic one
		entry = map.value("generic").data();

	if(!entry)	// Then use the default one
		entry = map.value("default").data();

	// If a default one doesn't exist, we don't know which one to use
	if(entry)
		return entry->creator.data();

	return nullptr;
}

const DataWidgetFactory::DataWidgetEntry* DataWidgetFactory::getEntry(int fullType, QString widgetName) const
{
	return registry.value(fullType).value(widgetName).data();
}

QList<QString> DataWidgetFactory::getWidgetNames(int fullType) const
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
