#include <ui/DataWidgetFactory.h>

#include <panda/BaseData.h>
#include <panda/DataFactory.h>

#include <iostream>

DataWidgetFactory* DataWidgetFactory::getInstance()
{
	static DataWidgetFactory instance;
	return &instance;
}

BaseDataWidget* DataWidgetFactory::create(panda::BaseData* data, QWidget* parent)
{
	int fullType = data->getDataTrait()->fullTypeId();
	QString widgetName = data->getWidget();
	DataWidgetEntry* entry = getEntry(fullType, widgetName);
	if(!entry)
	{
		entry = getEntry(fullType, "default");
		if(entry)
			std::cerr << "No widget named " << widgetName.toStdString() << ", using the default one." << std::endl;
		else
			std::cerr << "No widget for the type " << panda::DataFactory::typeToName(fullType).toStdString() << std::endl;
	}

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
