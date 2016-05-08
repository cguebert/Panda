#pragma once

#include <panda/messaging.h>

#include <memory>
#include <vector>

namespace panda {

class PandaObject;

class PANDA_CORE_API ObjectsList
{
public:
	using ObjectSPtr = std::shared_ptr<PandaObject>;
	using SPtrList = std::vector<ObjectSPtr>;

	void addObject(ObjectSPtr object);
	void removeObject(PandaObject* object);
	void clear(bool runSignals = true);

	int size() const;
	const SPtrList& get() const;
	ObjectSPtr getShared(PandaObject* object) const;

	PandaObject* find(uint32_t objectIndex);

	int getObjectPosition(PandaObject* object) const; /// Get the object's position in the objects list
	void reinsertObject(PandaObject* object, int pos); /// Reorder the object so it is inserted at the index pos in the objects list

	msg::Signal<void(panda::PandaObject*)> addedObject;
	msg::Signal<void(panda::PandaObject*)> removedObject;
	msg::Signal<void()> reorderedObjects;
	msg::Signal<void()> clearedList;

private:
	SPtrList m_objects;
};

//****************************************************************************//

inline int ObjectsList::size() const
{ return m_objects.size(); }

inline const ObjectsList::SPtrList& ObjectsList::get() const
{ return m_objects; }

} // namespace panda

