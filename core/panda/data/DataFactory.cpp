#include <panda/data/DataFactory.h>
#include <panda/helper/algorithm.h>

#include <iostream>

namespace panda
{

DataFactory& DataFactory::instance()
{
	static DataFactory instance;
	return instance;
}

const DataFactory::DataEntry* DataFactory::entry(const std::string& className)
{
	return instance().m_registry.at(className);
}

const DataFactory::DataEntry* DataFactory::entry(int type)
{
	return instance().m_typeRegistry.at(type);
}

std::shared_ptr<BaseData> DataFactory::create(const std::string& className, const std::string& name, const std::string& help, PandaObject* owner)
{
	if (instance().m_registry.count(className))
	{
		auto data = instance().m_registry.at(className)->creator->create(name, help, owner);
		data->setDynamicallyCreated(true);
		return data;
	}

	std::cerr << "Data factory has no entry for " << className << std::endl;
	return std::shared_ptr<BaseData>();
}

std::shared_ptr<BaseData> DataFactory::create(int type, const std::string& name, const std::string& help, PandaObject* owner)
{
	if (instance().m_typeRegistry.count(type))
	{
		auto data = instance().m_typeRegistry.at(type)->creator->create(name, help, owner);
		data->setDynamicallyCreated(true);
		return data;
	}

	std::cerr << "Data factory has no entry for type " << type << std::endl;
	return std::shared_ptr<BaseData>();
}

void DataFactory::registerData(types::AbstractDataTrait* dataTrait, const BaseClass* theClass, std::shared_ptr<BaseDataCreator> creator)
{
	std::string className = theClass->getTemplateName();
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

std::string DataFactory::typeToName(int type)
{
	const DataFactory::DataEntry* entry = instance().entry(type);
	if(entry)
		return entry->typeName;
	return "unknown";
}

int DataFactory::nameToType(const std::string& name)
{
	const DataFactory::DataEntry* entry = helper::valueOrDefault(instance().m_nameRegistry, name);
	if(entry)
		return entry->fullType;
	return -1;
}

const DataFactory::EntriesList& DataFactory::entries() 
{ 
	return instance().m_entries; 
}

} // namespace panda
