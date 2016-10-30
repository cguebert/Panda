#pragma once

#include <panda/messaging.h>

#include <vector>

namespace panda
{

class ObjectsList;
class PandaDocument;
class PandaObject;

namespace graphview
{

class PANDA_CORE_API ObjectsSelection
{
public:
	ObjectsSelection(ObjectsList& objectsList);

	using Objects = std::vector<PandaObject*>;

	msg::Signal<void()> selectionChanged; // Any time the selection is changed
	msg::Signal<void(PandaObject*)> selectedObject; // This is the last object to be added to the selection (we will show its properties in the GUI)

	void add(PandaObject* object);
	void remove(PandaObject* object);
	void clear();

	const Objects& get() const;
	void set(const Objects& selection);

	void selectOne(PandaObject* object);
	void selectNone();
	void selectAll();
	void selectConnected();

	bool isSelected(PandaObject* object) const;

	PandaObject* lastSelectedObject() const; // This is the object for which we show the properties in the GUI
	void setLastSelectedObject(PandaObject* object);

protected:
	Objects m_selectedObjects;
	ObjectsList& m_objectsList;
	msg::Observer m_observer; // Used to connect to signals (and disconnect automatically on destruction)
};

//****************************************************************************//

inline bool ObjectsSelection::isSelected(PandaObject* object) const
{ return std::find(m_selectedObjects.begin(), m_selectedObjects.end(), object) != m_selectedObjects.end(); }

inline const ObjectsSelection::Objects& ObjectsSelection::get() const
{ return m_selectedObjects; }

} // namespace graphview

} // namespace panda
