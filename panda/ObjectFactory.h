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

    typedef QMapIterator< QString, QSharedPointer<ClassEntry> > RegistryMapIterator;
    RegistryMapIterator getRegistryIterator() { return RegistryMapIterator(registry); }
protected:
    typedef QMap< QString, QSharedPointer<ClassEntry> > RegistryMap;
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

class RegisterObject
{
public:
    explicit RegisterObject(QString menuDisplay);

    RegisterObject& setDescription(QString description);
    RegisterObject& setName(QString name);
    RegisterObject& setHidden(bool hid);

    template <class T> RegisterObject& setClass()
    {
        entry.creator = QSharedPointer<BaseObjectCreator>(new ObjectCreator<T>);
		entry.theClass = T::getClass();
        return *this;
    }

    operator int();

protected:
    ObjectFactory::ClassEntry entry;

private:
    RegisterObject();
};

} // namespace panda

#endif // OBJECTFACTORY_H
