#include <panda/DataFactory.h>

#include <iostream>

namespace panda
{

DataFactory* DataFactory::getInstance()
{
	static DataFactory instance;
	return &instance;
}

const DataFactory::DataEntry* DataFactory::getEntry(QString className) const
{
	return registry.value(className);
}

const DataFactory::DataEntry* DataFactory::getEntry(int type) const
{
	return typeRegistry.value(type);
}

BaseData* DataFactory::create(QString className, const QString& name, const QString& help, PandaObject* owner)
{
	if(registry.contains(className))
		return registry.value(className)->creator->create(name, help, owner);

	std::cerr << "Data factory has no entry for " << className.toStdString() << std::endl;
	return nullptr;
}

BaseData* DataFactory::create(int type, const QString& name, const QString& help, PandaObject* owner)
{
	if(typeRegistry.contains(type))
		return typeRegistry.value(type)->creator->create(name, help, owner);

	std::cerr << "Data factory has no entry for type " << type << std::endl;
	return nullptr;
}

void DataFactory::registerData(QString description, int fullType, const BaseClass* theClass, QSharedPointer<BaseDataCreator> creator)
{
	QString typeName = theClass->getTypeName();
	DataEntry* entry = registry.value(typeName);
	if(!entry)
	{
		QSharedPointer<DataEntry> newEntry = QSharedPointer<DataEntry>::create();
		entries.push_back(newEntry);
		entry = newEntry.data();
	}

	entry->description = description;
	entry->className = typeName;
	entry->creator = creator;
	entry->fullType = fullType;
	entry->theClass = theClass;

	registry[typeName] = entry;
	typeRegistry[fullType] = entry;
	descriptionRegistry[description] = entry;
}

QString DataFactory::typeToDescription(int type)
{
	const DataFactory::DataEntry* entry = DataFactory::getInstance()->getEntry(type);
	if(entry)
		return entry->description;
	return "unknown";
}

int DataFactory::descriptionToType(QString description)
{
	const DataFactory::DataEntry* entry = DataFactory::getInstance()->descriptionRegistry[description];
	if(entry)
		return entry->fullType;
	return -1;
}

} // namespace panda
