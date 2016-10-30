#include <panda/graphview/ObjectsSelection.h>

#include <panda/document/GraphUtils.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>

namespace panda
{

namespace graphview
{

ObjectsSelection::ObjectsSelection(ObjectsList& objectsList)
	: m_objectsList(objectsList)
{
	m_observer.get(m_objectsList.clearedList).connect<ObjectsSelection, &ObjectsSelection::clear>(this);
	m_observer.get(m_objectsList.removedObject).connect<ObjectsSelection, &ObjectsSelection::remove>(this);
}

void ObjectsSelection::clear()
{
	m_selectedObjects.clear();
	selectedObject.run(nullptr);
	selectionChanged.run();
}

PandaObject* ObjectsSelection::lastSelectedObject() const
{
	if(m_selectedObjects.empty())
		return nullptr;
	else
		return m_selectedObjects.back();
}

void ObjectsSelection::setLastSelectedObject(PandaObject* object)
{
	helper::removeAll(m_selectedObjects, object);
	m_selectedObjects.push_back(object);
	selectedObject.run(object);
	selectionChanged.run();
}

void ObjectsSelection::set(const Objects& selection)
{
	m_selectedObjects = selection;
	selectedObject.run(m_selectedObjects.empty() ? nullptr : m_selectedObjects.back());
	selectionChanged.run();
}

void ObjectsSelection::selectOne(PandaObject* object)
{
	if (!object)
		selectNone();
	else
	{
		m_selectedObjects.clear();
		m_selectedObjects.push_back(object);
	}

	selectedObject.run(m_selectedObjects.empty() ? nullptr : m_selectedObjects.back());
	selectionChanged.run();
}

void ObjectsSelection::add(PandaObject* object)
{
	if(!helper::contains(m_selectedObjects, object))
	{
		m_selectedObjects.push_back(object);
		selectedObject.run(object);
		selectionChanged.run();
	}
}

void ObjectsSelection::remove(PandaObject* object)
{
	if(helper::contains(m_selectedObjects, object))
	{
		helper::removeAll(m_selectedObjects, object);
		selectedObject.run(m_selectedObjects.empty() ? nullptr : m_selectedObjects.back());
		selectionChanged.run();
	}
}

void ObjectsSelection::selectAll()
{
	const auto& objects = m_objectsList.get();
	if (objects.empty())
		return;

	m_selectedObjects.clear();
	for(auto& object : objects)
		m_selectedObjects.push_back(object.get());
	selectedObject.run(m_selectedObjects.back());
	selectionChanged.run();
}

void ObjectsSelection::selectNone()
{
	if(!m_selectedObjects.empty())
	{
		m_selectedObjects.clear();
		selectedObject.run(nullptr);
		selectionChanged.run();
	}
}

void ObjectsSelection::selectConnected()
{
	if (m_selectedObjects.empty())
		return;

	auto currentSelected = m_selectedObjects.back();
	m_selectedObjects = graph::computeConnectedObjects(m_selectedObjects);
	setLastSelectedObject(currentSelected);
	selectionChanged.run();
}

} // namespace graphview

} // namespace panda
