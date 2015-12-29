#include <panda/ObjectFactory.h>
#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>

#include <panda/types/DataTypeId.h>
#include <panda/types/DataTraits.h>

#include <iostream>

namespace
{

using NamesReplacementList = std::vector<std::pair<QString, QString>>;

static NamesReplacementList& getNamesReplacementList()
{
	static NamesReplacementList list;
	if (list.empty())
	{
		auto types = panda::types::DataTypeId::getTypesList();
		for (auto type : types)
		{
			auto trait = panda::types::DataTraitsList::getTrait(type);
			auto typeName = panda::BaseClass::decodeTypeName(trait->typeInfo());
			auto replacement = trait->typeName();
			if (typeName != replacement)
				list.emplace_back("<"+typeName+">", "<"+replacement+">");
		}
	}
	return list;
}

QString replaceTypeNames(const QString& input)
{
	QString output = input;
	for (const auto& p : getNamesReplacementList())
		output.replace(p.first, p.second);
	return output;
}

}

namespace panda
{

static bool objectFactoryCreated = true;

ObjectFactory::~ObjectFactory()
{
	objectFactoryCreated = false;
}

ObjectFactory* ObjectFactory::getInstance()
{
	static ObjectFactory instance;
	return &instance;
}

QSharedPointer<PandaObject> ObjectFactory::create(QString className, PandaDocument* parent) const
{
	auto iter = m_registry.find(className);
	if(iter != m_registry.end())
	{
		ClassEntry entry = iter->second;
		if(entry.creator)
		{
			QSharedPointer<PandaObject> object = entry.creator->create(parent);
			if(object)
			{
				object->setInternalData(entry.objectName, parent->getNextIndex());
				object->postCreate();
			}
			return object;
		}
	}

	std::cerr << "Factory has no entry for " << className.toStdString() << std::endl;
	return QSharedPointer<PandaObject>();
}

QString ObjectFactory::getRegistryName(PandaObject* object)
{
	return replaceTypeNames(object->getClass()->getTypeName());
}

void ObjectFactory::registerObject(QString className, ClassEntry entry)
{
	entry.className = className;
	if(m_registry.find(className) != m_registry.end() || m_tempRegistry.find(className) != m_tempRegistry.end())
		std::cerr << "Factory already has an entry for " << className.toStdString() << std::endl;
	m_tempRegistry.emplace(className, entry);
}

void ObjectFactory::registerModule(ModuleEntry entry)
{
	if(std::find(m_modules.begin(), m_modules.end(), entry.name) != m_modules.end())
		std::cerr << "Factory already has the module " << entry.name.toStdString() << std::endl;
	m_tempModules.push_back(entry);
}

void ObjectFactory::unregisterModule(QString moduleName)
{
	if(std::find(m_modules.begin(), m_modules.end(), moduleName) == m_modules.end())
	{
		std::cerr << "Factory has no module " << moduleName.toStdString() << std::endl;
		return;
	}

	for(RegistryMap::iterator it = m_registry.begin(), itEnd = m_registry.end(); it != itEnd; )
	{
		if(it->second.moduleName == moduleName)
			m_registry.erase(it++);
		else
			++it;
	}
}

void ObjectFactory::moduleLoaded()
{
	if(m_tempModules.empty())
		std::cerr << "No registered module" << std::endl;
	else if(m_tempModules.size() > 1)
		std::cerr << "More than one module registered" << std::endl;
	else
	{
		QString moduleName = m_tempModules.front().name;
		for(auto& it : m_tempRegistry)
			it.second.moduleName = moduleName;
		m_registry.insert(m_tempRegistry.begin(), m_tempRegistry.end());
		m_modules.push_back(m_tempModules.front());
		std::sort(m_modules.begin(), m_modules.end());

		std::cout << "Module " << m_tempModules.front().name.toStdString() << " registered "
				  << m_tempRegistry.size() << " components" << std::endl;
	}
	m_tempRegistry.clear();
	m_tempModules.clear();
}

void ObjectFactory::allObjectsRegistered()
{
	auto registryCopy = m_registry;
	for (const auto &it : registryCopy)
	{
		const auto& name = it.first;
		auto alias = replaceTypeNames(name);
		if (name != alias)
			m_registry.emplace(alias, it.second);
	}
}

void objectDeletor(PandaObject* object)
{
	object->preDestruction();
	delete object;
}

//****************************************************************************//

RegisterModule::RegisterModule(QString moduleName)
{
	m_entry.name = moduleName;
}

RegisterModule& RegisterModule::setDescription(QString description)
{
	m_entry.description = description;
	return *this;
}

RegisterModule& RegisterModule::setLicense(QString license)
{
	m_entry.license = license;
	return *this;
}

RegisterModule& RegisterModule::setVersion(QString version)
{
	m_entry.version = version;
	return *this;
}

const ObjectFactory::ModuleEntry RegisterModule::getEntry() const
{
	return m_entry;
}

//****************************************************************************//

ModuleHandle::ModuleHandle(const RegisterModule& registerInfo)
	: m_entry(registerInfo.getEntry())
{
	ObjectFactory::getInstance()->registerModule(m_entry);
}

ModuleHandle::~ModuleHandle()
{
	if(objectFactoryCreated)
		ObjectFactory::getInstance()->unregisterModule(m_entry.name);
}

} // namespace panda
