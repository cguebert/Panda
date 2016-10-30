#include <panda/graphview/object/ObjectRendererFactory.h>

#include <panda/object/PandaObject.h>
#include <panda/messaging.h>
#include <panda/document/PandaDocument.h>
#include <panda/types/DataTraits.h>

namespace panda
{

namespace graphview
{

namespace object
{

ObjectRendererFactory* ObjectRendererFactory::getInstance()
{
	static ObjectRendererFactory factory;
	return &factory;
}

std::shared_ptr<ObjectRenderer> ObjectRendererFactory::createRenderer(GraphView* view, PandaObject* obj)
{
	for(const auto& creator : creators)
	{
		if(creator->getClass()->isInstance(obj))
			return creator->create(view, obj);
	}

	return std::make_shared<ObjectRenderer>(view, obj);
}

void ObjectRendererFactory::addCreator(BaseObjectDrawCreator* creator)
{
	const BaseClass* newClass = creator->getClass();
	std::shared_ptr<BaseObjectDrawCreator> ptr(creator);

	unsigned int nb = creators.size();
	for(unsigned int i=0; i<nb; ++i)
	{
		const BaseClass* prevClass = creators[i]->getClass();
		if(newClass->hasParent(prevClass))
		{
			creators.insert(creators.begin() + i, ptr);
			return;
		}
	}

	creators.push_back(ptr);
}

} // namespace object

} // namespace graphview

} // namespace panda
