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
	m_registry.emplace(className, entry);
}

void objectDeletor(PandaObject* object)
{
	object->preDestruction();
	delete object;
}

} // namespace panda
