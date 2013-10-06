#include <panda/ObjectFactory.h>
#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>

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

} // namespace panda
