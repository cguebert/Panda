#include <panda/ObjectFactory.h>
#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>

#include <iostream>

namespace panda
{

ObjectFactory* ObjectFactory::getInstance()
{
	static ObjectFactory instance;
	return &instance;
}

PandaObject* ObjectFactory::create(QString className, PandaDocument* parent) const
{
	if(registry.contains(className))
	{
		ClassEntry entry = registry.value(className);
		if(!entry.creator.isNull())
		{
			PandaObject* object = entry.creator->create(parent);
			if(object)
			{
				object->setInternalData(entry.objectName, parent->getNextIndex());
				object->postCreate();
			}
			return object;
		}
	}

	std::cerr << "Factory has no entry for " << className.toStdString() << std::endl;
	return nullptr;
}

QString ObjectFactory::getRegistryName(PandaObject* object)
{
	return object->getBaseClass()->getTypeName();
}

ObjectFactory::RegistryMapIterator ObjectFactory::getRegistryIterator() const
{
	return RegistryMapIterator(registry);
}

void ObjectFactory::registerObject(QString className, ClassEntry entry)
{
	entry.className = className;
	registry.insert(className, entry);
}

} // namespace panda
