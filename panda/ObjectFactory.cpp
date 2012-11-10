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

QString ObjectFactory::decodeTypeName(const std::type_info& type)
{
    QString name;
#ifdef __GNUC__
    int status;
    char* allocname = abi::__cxa_demangle(type.name(), 0, 0, &status);
    if(allocname)
        name = allocname;
    else
        std::cerr << "Unable to demangle symbol: " << type.name() << std::endl;
#else
    name = type.name();
	name.replace("class ", "");
#endif
    return name;
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
    return NULL;
}

ObjectFactory::ClassEntry* ObjectFactory::getEntry(QString className)
{
    if(!registry.contains(className))
    {
        QSharedPointer<ObjectFactory::ClassEntry> entry(new ObjectFactory::ClassEntry);
        registry.insert(className, entry);
        return entry.data();
    }

    return registry.value(className).data();
}

QString ObjectFactory::getRegistryName(PandaObject* object)
{
    return decodeTypeName(typeid(*object));
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

    ObjectFactory::ClassEntry* reg = ObjectFactory::getInstance()->getEntry(entry.className);
    reg->creator = entry.creator;
    reg->description = entry.description;
    reg->objectName = entry.objectName;
    reg->menuDisplay = entry.menuDisplay;
    reg->className = entry.className;
    reg->hidden = entry.hidden;

    return 1;
}

} // namespace panda
