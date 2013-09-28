#include <panda/ObjectFactory.h>
#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace panda
{

ObjectFactory* ObjectFactory::getInstance()
{
	static ObjectFactory instance;
	return &instance;
}

PandaObject* ObjectFactory::create(QString className, PandaDocument* parent)
{
	if(registry.contains(className))
	{
		QSharedPointer<ClassEntry> entry = registry.value(className);
		if(!entry->creator.isNull())
		{
			PandaObject* object = entry->creator->create(parent);
			if(object)
			{
				object->setInternalData(entry->objectName, parent->getNextIndex());
				object->postCreate();
			}
			return object;
		}
	}

	std::cerr << "Factory has no entry for " << className.toStdString() << std::endl;
	return nullptr;
}

ObjectFactory::ClassEntry* ObjectFactory::getEntry(QString className)
{
	if(!registry.contains(className))
	{
		ClassEntryPtr entry = ClassEntryPtr::create();
		registry.insert(className, entry);
		return entry.data();
	}

	return registry.value(className).data();
}

QString ObjectFactory::getRegistryName(PandaObject* object)
{
	return object->getBaseClass()->getTypeName();
}

RegisterObject::RegisterObject(QString menuDisplay)
{
	entry.menuDisplay = menuDisplay;
	if(!menuDisplay.isEmpty())
		entry.objectName = menuDisplay.split("/").last();
}

RegisterObject& RegisterObject::setDescription(QString description)
{
	entry.description = description;
	return *this;
}

RegisterObject& RegisterObject::setName(QString name)
{
	entry.objectName = name;
	return *this;
}

RegisterObject& RegisterObject::setHidden(bool hid)
{
	entry.hidden = hid;
	return *this;
}

RegisterObject::operator int()
{
	if(entry.creator.isNull())
	{
		std::cerr << "Error : missing creator in registry for " << entry.menuDisplay.toStdString() << std::endl;
		return 0;
	}

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

} // namespace panda
