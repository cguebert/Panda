#ifndef HELPER_FACTORY_H
#define HELPER_FACTORY_H

#include <panda/BaseClass.h>

#include <QString>
#include <QMultiMap>

namespace panda
{

namespace helper
{

QString getFactoryLog();
void logFactoryRegister(QString baseclass, QString classname, QString key, bool multi);

template <class Object, class Argument, class ObjectPtr = Object*>
class BaseCreator
{
public:
	virtual ~BaseCreator() { }
	virtual ObjectPtr createInstance(Argument arg) = 0;
	virtual const std::type_info& type() = 0;
};

template <class TObject, typename TArgument, typename TPtr = TObject* >
class Factory
{
public:
	typedef QString   Key;
	typedef TObject   Object;
	typedef TPtr	  ObjectPtr;
	typedef TArgument Argument;
	typedef BaseCreator<Object, Argument, ObjectPtr> Creator;
	typedef std::multimap<Key, Creator> Registry;

protected:
	QMultiMap<Key, Creator*> registry;

public:
	bool registerCreator(Key key, Creator* creator, bool multi=false)
	{
		if(!multi && registry.contains(key))
			return false;
		logFactoryRegister(BaseClass::decodeTypeName(typeid(Object)),
						   BaseClass::decodeTypeName(creator->type()),
						   key, multi);
		registry.insert(key, creator);
		return true;
	}

	ObjectPtr createObject(Key key, Argument arg)
	{
		for(Creator* creator : registry.values(key))
		{
			auto object = creator->createInstance(arg);
			if(object)
				return object;
		}
		return nullptr;
	}

	ObjectPtr createAnyObject(Argument arg)
	{
		for(Creator* creator : registry.values())
		{
			auto object = creator->createInstance(arg);
			if (object != nullptr)
				return object;
		}
		return nullptr;
	}

	QList<Key> uniqueKeys()
	{
		 return registry.keys();
	}

	bool hasKey(Key key)
	{
		return registry.contains(key);
	}

	bool resetEntry( Key existingKey)
	{
		return (registry.remove(existingKey) > 0);
	}

	static Factory<Object, Argument, ObjectPtr>* getInstance()
	{
		static Factory<Object, Argument, ObjectPtr> instance;
		return &instance;
	}

	static ObjectPtr CreateObject(Key key, Argument arg)
	{
		return getInstance()->createObject(key, arg);
	}

	static ObjectPtr CreateAnyObject(Argument arg)
	{
		return getInstance()->createAnyObject(arg);
	}

	static QList<Key> UniqueKeys()
	{
		return getInstance()->uniqueKeys();
	}

	static bool HasKey(Key key)
	{
		return getInstance()->hasKey(key);
	}

	static bool ResetEntry(Key existing)
	{
		return getInstance()->resetEntry(existing);
	}
};

template <class Factory, class RealObject>
class Creator : public Factory::Creator
{
public:
	typedef typename Factory::Object	Object;
	typedef typename Factory::ObjectPtr ObjectPtr;
	typedef typename Factory::Argument  Argument;
	typedef typename Factory::Key		Key;
	Creator(Key key, bool multi=false)
	{
		Factory::getInstance()->registerCreator(key, this, multi);
	}
	ObjectPtr createInstance(Argument arg)
	{
		RealObject* instance = nullptr;
		return RealObject::create(instance, arg);
	}
	const std::type_info& type()
	{
		return typeid(RealObject);
	}
};

template <class Factory, class RealObject>
class CreatorFn : public Factory::Creator
{
public:
	typedef typename Factory::Object	Object;
	typedef typename Factory::ObjectPtr ObjectPtr;
	typedef typename Factory::Argument  Argument;
	typedef typename Factory::Key		Key;
	typedef ObjectPtr Fn(RealObject* obj, Argument arg);
	Fn* constructor;

	CreatorFn(Key key, Fn* constructor, bool multi=false)
		: constructor(constructor)
	{
		Factory::getInstance()->registerCreator(key, this, multi);
	}

	ObjectPtr createInstance(Argument arg)
	{
		RealObject* instance = nullptr;
		return (*constructor)(instance, arg);
	}
	const std::type_info& type()
	{
		return typeid(RealObject);
	}
};


} // namespace helper

} // namespace panda

// Creator is often used without namespace qualifiers
using panda::helper::Creator;

#endif
