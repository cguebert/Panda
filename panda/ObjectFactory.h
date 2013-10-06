#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <panda/BaseClass.h>

#include <QSharedPointer>
#include <QMap>
#include <iostream>
#include <typeinfo>

namespace panda
{

class PandaObject;
class PandaDocument;

class BaseObjectCreator
{
public:
	virtual ~BaseObjectCreator() {}
	virtual PandaObject* create(PandaDocument* parent) = 0;
};

class ObjectFactory
{
private:
	ObjectFactory() {}

public:
	class ClassEntry
	{
	public:
		ClassEntry() : hidden(false) {}

		QString menuDisplay;
		QString objectName;
		QString description;
		QString className;
		const BaseClass* theClass;
		QSharedPointer<BaseObjectCreator> creator;
		bool hidden;
	};

	static ObjectFactory* getInstance();
	ClassEntry* getEntry(QString className);

	template <class T>
	static QString getRegistryName()
	{
		return T::getClass()->getTypeName();
	}
	static QString getRegistryName(PandaObject* object);

	PandaObject* create(QString className, PandaDocument* parent);

	typedef QSharedPointer<ClassEntry> ClassEntryPtr;
	typedef QMapIterator< QString, ClassEntryPtr > RegistryMapIterator;
	RegistryMapIterator getRegistryIterator() { return RegistryMapIterator(registry); }
protected:
	typedef QMap< QString, ClassEntryPtr > RegistryMap;
	RegistryMap registry;
};

template<class T>
class ObjectCreator : public BaseObjectCreator
{
public:
	virtual PandaObject* create(PandaDocument* parent)
	{
		return new T(parent);
	}
};

template <class T>
class RegisterObject
{
public:
	explicit RegisterObject(QString menuDisplay)
	{
		entry.creator = QSharedPointer<ObjectCreator<T>>::create();
		entry.theClass = T::getClass();
		entry.menuDisplay = menuDisplay;
		if(!menuDisplay.isEmpty())
			entry.objectName = menuDisplay.split("/").last();
	}

	RegisterObject& setDescription(QString description)
	{
		entry.description = description;
		return *this;
	}

	RegisterObject& setName(QString name)
	{
		entry.objectName = name;
		return *this;
	}

	RegisterObject& setHidden(bool hid)
	{
		entry.hidden = hid;
		return *this;
	}

	operator int()
	{
		QString typeName = entry.theClass->getTypeName();
		ObjectFactory::ClassEntry* reg = ObjectFactory::getInstance()->getEntry(typeName);
		reg->creator = entry.creator;
		reg->description = entry.description;
		reg->objectName = entry.objectName;
		reg->menuDisplay = entry.menuDisplay;
		reg->theClass = entry.theClass;
		reg->className = typeName;
		reg->hidden = entry.hidden;

		return 1;
	}

protected:
	ObjectFactory::ClassEntry entry;

private:
	RegisterObject();
};

} // namespace panda

#endif // OBJECTFACTORY_H
