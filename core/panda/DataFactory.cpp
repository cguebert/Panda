#include <panda/DataFactory.h>
#include <panda/helper/algorithm.h>

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
	return m_registry.at(className);
}

const DataFactory::DataEntry* DataFactory::getEntry(int type) const
{
	return m_typeRegistry.at(type);
}

std::shared_ptr<BaseData> DataFactory::create(QString className, const QString& name, const QString& help, PandaObject* owner) const
{
	if (m_registry.count(className))
		return m_registry.at(className)->creator->create(name, help, owner);

	std::cerr << "Data factory has no entry for " << className.toStdString() << std::endl;
	return std::shared_ptr<BaseData>();
}

std::shared_ptr<BaseData> DataFactory::create(int type, const QString& name, const QString& help, PandaObject* owner) const
{
	if (m_typeRegistry.count(type))
		return m_typeRegistry.at(type)->creator->create(name, help, owner);

	std::cerr << "Data factory has no entry for type " << type << std::endl;
	return std::shared_ptr<BaseData>();
}

void DataFactory::registerData(types::AbstractDataTrait* dataTrait, const BaseClass* theClass, std::shared_ptr<BaseDataCreator> creator)
{
	QString className = theClass->getTemplateName();
	DataEntry* entry = panda::helper::valueOrDefault(m_registry, className, nullptr);
	if(!entry)
	{
		std::shared_ptr<DataEntry> newEntry = std::make_shared<DataEntry>();
		m_entries.push_back(newEntry);
		entry = newEntry.get();
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
