#include <ui/graphview/ObjectsSelection.h>

#include <panda/PandaDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/GraphUtils.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>

ObjectsSelection::ObjectsSelection(panda::PandaDocument* document)
	: m_document(document)
{
	m_observer.get(document->getSignals().clearDocument).connect<ObjectsSelection, &ObjectsSelection::clear>(this);
	m_observer.get(document->getSignals().removedObject).connect<ObjectsSelection, &ObjectsSelection::remove>(this);
}

void ObjectsSelection::clear()
{
	m_selectedObjects.clear();
	selectedObject.run(nullptr);
	selectionChanged.run();
}

panda::PandaObject* ObjectsSelection::lastSelectedObject() const
{
	if(m_selectedObjects.empty())
		return nullptr;
	else
		return m_selectedObjects.back();
}

void ObjectsSelection::setLastSelectedObject(panda::PandaObject* object)
{
	panda::helper::removeAll(m_selectedObjects, object);
	m_selectedObjects.push_back(object);
	selectedObject.run(object);
	selectionChanged.run();
}

void ObjectsSelection::set(const ObjectsList& selection)
{
	m_selectedObjects = selection;
	selectedObject.run(m_selectedObjects.empty() ? nullptr : m_selectedObjects.back());
	selectionChanged.run();
}

void ObjectsSelection::add(panda::PandaObject* object)
{
	if(!panda::helper::contains(m_selectedObjects, object))
	{
		m_selectedObjects.push_back(object);
		selectedObject.run(object);
		selectionChanged.run();
	}
}

void ObjectsSelection::remove(panda::PandaObject* object)
{
	if(panda::helper::contains(m_selectedObjects, object))
	{
		panda::helper::removeAll(m_selectedObjects, object);
		selectedObject.run(m_selectedObjects.empty() ? nullptr : m_selectedObjects.back());
		selectionChanged.run();
	}
}

void ObjectsSelection::selectAll()
{
	const auto& objects = m_document->getObjects();
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
	m_selectedObjects = panda::graph::computeConnectedObjects(m_selectedObjects);
	setLastSelectedObject(currentSelected);
	selectionChanged.run();
}
