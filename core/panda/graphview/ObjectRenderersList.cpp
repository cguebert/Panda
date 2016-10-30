#include <panda/graphview/ObjectRenderersList.h>
#include <panda/graphview/object/ObjectRenderer.h>

#include <panda/helper/algorithm.h>
#include <panda/document/ObjectsList.h>

namespace panda
{

namespace graphview
{

	object::ObjectRenderer* ObjectRenderersList::get(PandaObject* object) const
	{
		return helper::valueOrDefault(m_objectRenderers, object).get();
	}

	ObjectRenderersList::ObjectRendererSPtr ObjectRenderersList::getSPtr(PandaObject* object) const
	{
		return helper::valueOrDefault(m_objectRenderers, object);
	}

	ObjectRenderersList::ObjectRenderers ObjectRenderersList::get(const Objects& objects) const
	{
		ObjectRenderers list;
		for (auto object : objects)
		{
			auto objRnd = get(object);
			if(objRnd)
				list.push_back(objRnd);
		}

		return list;
	}

	object::ObjectRenderer* ObjectRenderersList::getAtPos(const types::Point& pt) const
	{
		for (auto it = m_orderedObjectRenderers.rbegin(); it != m_orderedObjectRenderers.rend(); ++it)
		{
			if((*it)->contains(pt))
				return *it;
		}
		return nullptr;
	}

	void ObjectRenderersList::set(PandaObject* object, const ObjectRendererSPtr& drawStruct)
	{
		m_objectRenderers[object] = drawStruct;
		m_orderedObjectRenderers.push_back(drawStruct.get());
	}

	void ObjectRenderersList::remove(PandaObject* object)
	{
		auto objRnd = get(object);
		m_objectRenderers.erase(object);
		if(objRnd)
			helper::removeOne(m_orderedObjectRenderers, objRnd);
	}

	void ObjectRenderersList::reorder(const ObjectsList& objects)
	{
		m_orderedObjectRenderers.clear();
		for (const auto& obj : objects.get())
			m_orderedObjectRenderers.push_back(get(obj.get()));
	}

	void ObjectRenderersList::initializeRenderer(ViewRenderer& viewRenderer)
	{
		for (const auto& objRnd : m_orderedObjectRenderers)
			objRnd->initializeRenderer(viewRenderer);
	}

} // namespace graphview

} // namespace panda
