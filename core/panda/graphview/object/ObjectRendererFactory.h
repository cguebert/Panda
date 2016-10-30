#pragma once

#include <panda/graphview/object/ObjectRenderer.h>

#include <memory>

namespace panda
{
	class PandaObject;
	class BaseData;
	class BaseClass;
	class XmlElement;

	namespace msg
	{ class Observer; }
}

namespace graphview
{

namespace graphics {
	struct DrawColors;
}

class GraphView;
class MouseEvent;
class ObjectPositionAddon;
class ViewRenderer;

namespace object
{

class PANDA_CORE_API BaseObjectDrawCreator
{
public:
	virtual ~BaseObjectDrawCreator() {}
	virtual const panda::BaseClass* getClass() const = 0;
	virtual std::shared_ptr<ObjectRenderer> create(GraphView* view, panda::PandaObject* obj) const = 0;
};

template<class O, class D>
class ObjectDrawCreator : public BaseObjectDrawCreator
{
public:
	ObjectDrawCreator()
		: theClass(O::GetClass())
	{ }

	const panda::BaseClass* getClass() const override
	{ return theClass; }

	std::shared_ptr<ObjectRenderer> create(GraphView* view, panda::PandaObject* obj) const override
	{ return std::make_shared<D>(view, dynamic_cast<O*>(obj)); }

protected:
	const panda::BaseClass* theClass;
};

class PANDA_CORE_API ObjectRendererFactory
{
public:
	static ObjectRendererFactory* getInstance();
	std::shared_ptr<ObjectRenderer> createRenderer(GraphView *view, panda::PandaObject *obj);

protected:
	void addCreator(BaseObjectDrawCreator* creator);

	std::vector< std::shared_ptr<BaseObjectDrawCreator> > creators;

	template <class O, class D> friend class RegisterDrawObject;
};

template <class O, class D>
class RegisterDrawObject
{
public:
	RegisterDrawObject() {}
	operator int()
	{
		ObjectRendererFactory::getInstance()->addCreator(new ObjectDrawCreator<O, D>());
		return 1;
	}
};

} // namespace object

} // namespace graphview
