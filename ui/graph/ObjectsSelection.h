#pragma once

#include <panda/messaging.h>

#include <vector>

namespace panda
{
class PandaDocument;
class PandaObject;
}

class ObjectsSelection
{
public:
	ObjectsSelection(panda::PandaDocument* document);

	using ObjectsList = std::vector<panda::PandaObject*>;

	panda::msg::Signal<void()> selectionChanged; // Any time the selection is changed
	panda::msg::Signal<void(panda::PandaObject*)> selectedObject; // This is the last object to be added to the selection (we will show its properties in the GUI)

	void add(panda::PandaObject* object);
	void remove(panda::PandaObject* object);
	void clear();

	const ObjectsList& get() const;
	void set(const ObjectsList& selection);

	void selectNone();
	void selectAll();
	void selectConnected();

	bool isSelected(panda::PandaObject* object) const;

	panda::PandaObject* lastSelectedObject() const; // This is the object for which we show the properties in the GUI
	void setLastSelectedObject(panda::PandaObject* object);

protected:
	ObjectsList m_selectedObjects;
	panda::PandaDocument* m_document;
	panda::msg::Observer m_observer; // Used to connect to signals (and disconnect automatically on destruction)
};

//****************************************************************************//

inline bool ObjectsSelection::isSelected(panda::PandaObject* object) const
{ return std::find(m_selectedObjects.begin(), m_selectedObjects.end(), object) != m_selectedObjects.end(); }

inline const ObjectsSelection::ObjectsList& ObjectsSelection::get() const
{ return m_selectedObjects; }
