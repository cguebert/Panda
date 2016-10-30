#pragma once

#include <panda/core.h>

#include <map>
#include <memory>
#include <vector>

namespace panda
{

class ObjectsList;
class PandaObject;

namespace types {
	class Point;
}

namespace graphview
{

	namespace object {
		class ObjectRenderer;
	}

	class ViewRenderer;

	class PANDA_CORE_API ObjectRenderersList
	{
	public:
		using Objects = std::vector<PandaObject*>;
		using ObjectRendererSPtr = std::shared_ptr<object::ObjectRenderer>;
		using ObjectRenderers = std::vector<object::ObjectRenderer*>;

		object::ObjectRenderer* get(PandaObject* object) const;
		ObjectRendererSPtr getSPtr(PandaObject* object) const;

		ObjectRenderers get(const Objects& objects) const;

		object::ObjectRenderer* getAtPos(const types::Point& pt) const;

		void set(PandaObject* object, const ObjectRendererSPtr& drawStruct);
		void remove(PandaObject* object);

		const ObjectRenderers& getOrdered() const;
		void reorder(const ObjectsList& objects);

		void initializeRenderer(ViewRenderer& viewRenderer);

	private:
		std::map<PandaObject*, ObjectRendererSPtr> m_objectRenderers;
		std::vector<object::ObjectRenderer*> m_orderedObjectRenderers; // In the same order as the document
	};

//****************************************************************************//

	inline const ObjectRenderersList::ObjectRenderers& ObjectRenderersList::getOrdered() const
	{ return m_orderedObjectRenderers; }

} // namespace graphview

} // namespace panda
