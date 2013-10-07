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

void DataFactory::registerData(types::AbstractDataTrait* dataTrait, const BaseClass* theClass, QSharedPointer<BaseDataCreator> creator)
{
	QString className = theClass->getTemplateName();
	DataEntry* entry = registry.value(className);
	if(!entry)
	{
		QSharedPointer<DataEntry> newEntry = QSharedPointer<DataEntry>::create();
		entries.push_back(newEntry);
		entry = newEntry.data();
	}

	entry->typeName = dataTrait->description();
	entry->className = className;
	entry->creator = creator;
	entry->fullType = dataTrait->fullTypeId();
	entry->theClass = theClass;

//	std::cout << typeName.toStdString() << "\t" << className.toStdString() << "\t" << fullType << std::endl;

	registry[className] = entry;
	typeRegistry[dataTrait->fullTypeId()] = entry;
	nameRegistry[dataTrait->description()] = entry;
}

QString DataFactory::typeToName(int type)
{
	const DataFactory::DataEntry* entry = DataFactory::getInstance()->getEntry(type);
	if(entry)
		return entry->typeName;
	return "unknown";
}

int DataFactory::nameToType(QString name)
{
	const DataFactory::DataEntry* entry = DataFactory::getInstance()->nameRegistry[name];
	if(entry)
		return entry->fullType;
	return -1;
}

} // namespace panda
