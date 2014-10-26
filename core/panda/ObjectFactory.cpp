#include <panda/ObjectFactory.h>
#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>

#include <iostream>

namespace panda
{

static bool objectFactoryCreated = true;

ObjectFactory::~ObjectFactory()
{
	objectFactoryCreated = false;
	std::cout << "ObjectFactory destruction" << std::endl;
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
	return object->getBaseClass()->getTypeName();
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
	std::cout << "registerModule " << entry.name.toStdString() << std::endl;
}

void ObjectFactory::unregisterModule(QString moduleName)
{
	if(std::find(m_modules.begin(), m_modules.end(), moduleName) != m_modules.end())
	{
		std::cerr << "Factory has no module " << moduleName.toStdString() << std::endl;
		return;
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
		m_registry.insert(m_tempRegistry.begin(), m_tempRegistry.end());
		m_modules.push_back(m_tempModules.front());
		std::sort(m_modules.begin(), m_modules.end());

		std::cout << "Module " << m_tempModules.front().name.toStdString() << " registered "
				  << m_tempRegistry.size() << " components" << std::endl;
	}
	m_tempRegistry.clear();
	m_tempModules.clear();
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
