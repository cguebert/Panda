#pragma once

#include <map>
#include <memory>
#include <vector>

namespace panda {
	class ObjectsList;
	class PandaObject;

	namespace types {
		class Point;
	}
}

namespace graphview
{

	namespace object {
		class ObjectRenderer;
	}

	class ObjectRenderersList
	{
	public:
		using Objects = std::vector<panda::PandaObject*>;
		using ObjectRendererSPtr = std::shared_ptr<object::ObjectRenderer>;
		using ObjectRenderers = std::vector<object::ObjectRenderer*>;

		object::ObjectRenderer* get(panda::PandaObject* object) const;
		ObjectRendererSPtr getSPtr(panda::PandaObject* object) const;

		ObjectRenderers get(const Objects& objects) const;

		object::ObjectRenderer* getAtPos(const panda::types::Point& pt) const;

		void set(panda::PandaObject* object, const ObjectRendererSPtr& drawStruct);
		void remove(panda::PandaObject* object);

		const ObjectRenderers& getOrdered() const;
		void reorder(const panda::ObjectsList& objects);

	private:
		std::map<panda::PandaObject*, ObjectRendererSPtr> m_objectRenderers;
		std::vector<object::ObjectRenderer*> m_orderedObjectRenderers; // In the same order as the document
	};

//****************************************************************************//

	inline const ObjectRenderersList::ObjectRenderers& ObjectRenderersList::getOrdered() const
	{ return m_orderedObjectRenderers; }

}
