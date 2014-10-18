#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <panda/BaseClass.h>

#include <QStringList>
#include <QSharedPointer>
#include <map>

namespace panda
{

class PandaObject;
class PandaDocument;

class PANDA_CORE_API BaseObjectCreator
{
public:
	virtual ~BaseObjectCreator() {}
	virtual QSharedPointer<PandaObject> create(PandaDocument* parent) = 0;
};

class PANDA_CORE_API ObjectFactory
{
private:
	ObjectFactory() {}

public:
	class ClassEntry
	{
	public:
		ClassEntry() : hidden(false), theClass(nullptr) {}

		QString menuDisplay;
		QString objectName;
		QString description;
		QString className;
		const BaseClass* theClass;
		QSharedPointer<BaseObjectCreator> creator;
		bool hidden;
	};

	static ObjectFactory* getInstance();

	template <class T>
	static QString getRegistryName()
	{ return T::getClass()->getTypeName(); }
	static QString getRegistryName(PandaObject* object);

	QSharedPointer<PandaObject> create(QString className, PandaDocument* parent) const;

	typedef std::map< QString, ClassEntry > RegistryMap;
	const RegistryMap& getRegistryMap() const
	{ return m_registry; }
protected:
	template<class T> friend class RegisterObject;
	void registerObject(QString className, ClassEntry entry);

	RegistryMap m_registry;
};

PANDA_CORE_API void objectDeletor(PandaObject* object);

template<class T>
class ObjectCreator : public BaseObjectCreator
{
public:
	virtual QSharedPointer<PandaObject> create(PandaDocument* parent)
	{
		return QSharedPointer<PandaObject>(new T(parent), objectDeletor);
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
		ObjectFactory::getInstance()->registerObject(typeName, entry);

		return 1;
	}

protected:
	ObjectFactory::ClassEntry entry;

private:
	RegisterObject();
};

} // namespace panda

#endif // OBJECTFACTORY_H