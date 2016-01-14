#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <panda/data/BaseClass.h>

#include <map>
#include <memory>

namespace panda
{

class PandaObject;
class PandaDocument;

class PANDA_CORE_API BaseObjectCreator
{
public:
	virtual ~BaseObjectCreator() {}
	virtual std::shared_ptr<PandaObject> create(PandaDocument* parent) = 0;
};

class PANDA_CORE_API ObjectFactory
{
public:
	struct ClassEntry
	{
		ClassEntry() : hidden(false), theClass(nullptr) {}

		std::string menuDisplay;
		std::string objectName;
		std::string description;
		std::string className;
		std::string moduleName;
		const BaseClass* theClass;
		std::shared_ptr<BaseObjectCreator> creator;
		bool hidden;
	};

	struct ModuleEntry
	{
		std::string name;
		std::string description;
		std::string license;
		std::string version;

		bool operator==(const std::string& n) const { return name == n; }
		bool operator<(const ModuleEntry& e) const { return name < e.name; }
	};

	~ObjectFactory();
	static ObjectFactory* getInstance();

	template <class T>
	static std::string getRegistryName()
	{ return T::GetClass()->getTypeName(); }
	static std::string getRegistryName(PandaObject* object);

	std::shared_ptr<PandaObject> create(const std::string& className, PandaDocument* parent) const;

	typedef std::map< std::string, ClassEntry > RegistryMap;
	const RegistryMap& getRegistryMap() const
	{ return m_registry; }

	typedef std::vector<ModuleEntry> ModulesList;
	const ModulesList& getModules() const
	{ return m_modules; }

protected:
	template<class T> friend class RegisterObject;
	void registerObject(const std::string& className, ClassEntry entry);

	friend class ModuleHandle;
	void registerModule(ModuleEntry entry);
	void unregisterModule(const std::string& moduleName);

	friend class PluginsManager;
	void moduleLoaded(); /// Copy the temporary list into the main one, and modify their module
	void allObjectsRegistered();

	RegistryMap m_registry, m_tempRegistry;
	ModulesList m_modules, m_tempModules;

private:
	ObjectFactory() {}
};

PANDA_CORE_API void objectDeletor(PandaObject* object);

//****************************************************************************//

template<class T>
class ObjectCreator : public BaseObjectCreator
{
public:
	virtual std::shared_ptr<PandaObject> create(PandaDocument* parent)
	{ return std::shared_ptr<PandaObject>(new T(parent), objectDeletor); }
};

template <class T>
class RegisterObject
{
public:
	explicit RegisterObject(std::string menuDisplay)
	{
		entry.creator = std::make_shared<ObjectCreator<T>>();
		entry.theClass = T::GetClass();
		entry.menuDisplay = menuDisplay;
		if (!menuDisplay.empty())
		{
			auto it = menuDisplay.rfind('/');
			if (it != std::string::npos)
				entry.objectName = menuDisplay.substr(it + 1);
			else
				entry.objectName = menuDisplay;
		}
	}

	RegisterObject& setDescription(std::string description)
	{ entry.description = description; return *this; }

	RegisterObject& setName(std::string name)
	{ entry.objectName = name; return *this; }

	RegisterObject& setHidden(bool hid)
	{ entry.hidden = hid; return *this; }

	operator int()
	{
		std::string typeName = entry.theClass->getTypeName();
		ObjectFactory::getInstance()->registerObject(typeName, entry);

		return 1;
	}

protected:
	ObjectFactory::ClassEntry entry;

private:
	RegisterObject();
};

//****************************************************************************//

/// Use the REGISTER_MODULE macro to be sure to use the real name of the target file

class PANDA_CORE_API RegisterModule
{
public:
	explicit RegisterModule(std::string moduleName);
	RegisterModule& setDescription(std::string description);
	RegisterModule& setLicense(std::string license);
	RegisterModule& setVersion(std::string version);

protected:
	ObjectFactory::ModuleEntry m_entry;

	friend class ModuleHandle;

private:
	RegisterModule();
};

#define EXPAND_MACRO(x) STRINGIFY_MACRO(x)
#define STRINGIFY_MACRO(x) #x
#define REGISTER_MODULE panda::RegisterModule(EXPAND_MACRO(TARGET))

// This class is used by RegisterModule to automatically unregister a module when unloading a library
class PANDA_CORE_API ModuleHandle
{
public:
	ModuleHandle(const RegisterModule& registerInfo);
	~ModuleHandle();

	ObjectFactory::ModuleEntry m_entry;

private:
	ModuleHandle() = delete;
};

} // namespace panda

#endif // OBJECTFACTORY_H
