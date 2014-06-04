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
	return m_registry.value(className);
}

const DataFactory::DataEntry* DataFactory::getEntry(int type) const
{
	return m_typeRegistry.value(type);
}

QSharedPointer<BaseData> DataFactory::create(QString className, const QString& name, const QString& help, PandaObject* owner) const
{
	if(m_registry.contains(className))
		return m_registry.value(className)->creator->create(name, help, owner);

	std::cerr << "Data factory has no entry for " << className.toStdString() << std::endl;
	return QSharedPointer<BaseData>();
}

QSharedPointer<BaseData> DataFactory::create(int type, const QString& name, const QString& help, PandaObject* owner) const
{
	if(m_typeRegistry.contains(type))
		return m_typeRegistry.value(type)->creator->create(name, help, owner);

	std::cerr << "Data factory has no entry for type " << type << std::endl;
	return QSharedPointer<BaseData>();
}

void DataFactory::registerData(types::AbstractDataTrait* dataTrait, const BaseClass* theClass, QSharedPointer<BaseDataCreator> creator)
{
	QString className = theClass->getTemplateName();
	DataEntry* entry = m_registry.value(className);
	if(!entry)
	{
		QSharedPointer<DataEntry> newEntry = QSharedPointer<DataEntry>::create();
		m_entries.push_back(newEntry);
		entry = newEntry.data();
	}

	entry->typeName = dataTrait->typeName();
	entry->className = className;
	entry->creator = creator;
	entry->fullType = dataTrait->fullTypeId();
	entry->theClass = theClass;

	m_registry[className] = entry;
	m_typeRegistry[dataTrait->fullTypeId()] = entry;
	m_nameRegistry[dataTrait->typeName()] = entry;
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
	const DataFactory::DataEntry* entry = DataFactory::getInstance()->m_nameRegistry[name];
	if(entry)
		return entry->fullType;
	return -1;
}

} // namespace panda
