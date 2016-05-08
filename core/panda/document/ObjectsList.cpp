#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>

namespace
{
	inline panda::ObjectsList::SPtrList::const_iterator findSPtr(const panda::ObjectsList::SPtrList& objects, panda::PandaObject* object)
	{
		return std::find_if(objects.begin(), objects.end(), [object](const auto& ptr){
			return ptr.get() == object;
		});
	}
}

namespace panda 
{

void ObjectsList::addObject(ObjectSPtr object)
{
	m_objects.push_back(object);
	object->addedToDocument();
	addedObject.run(object.get());
}

void ObjectsList::removeObject(PandaObject* object)
{
	object->removedFromDocument();
	removedObject.run(object);
	helper::removeIf(m_objects, [object](const auto& ptr){
		return ptr.get() == object;
	});
}

void ObjectsList::clear(bool runSignals)
{
	clearedList.run();
	for(auto object : m_objects)
	{
		if (runSignals)
			removedObject.run(object.get());
		object->preDestruction();
	}

	m_objects.clear();
}
	
ObjectsList::ObjectSPtr ObjectsList::getShared(PandaObject* object) const
{
	auto iter = findSPtr(m_objects, object);
	if(iter != m_objects.end())
		return *iter;

	return nullptr;
}

int ObjectsList::getObjectPosition(PandaObject* object) const
{
	auto iter = findSPtr(m_objects, object);
	if(iter != m_objects.end())
		return std::distance(m_objects.begin(), iter);

	return -1;
}

void ObjectsList::reinsertObject(PandaObject* object, int pos)
{
	auto iter = findSPtr(m_objects, object);
	if(iter == m_objects.end())
		return;

	int oldPos = iter - m_objects.begin();
	ObjectSPtr objectPtr = *iter;

	m_objects.erase(iter);
	if (pos == -1)
		pos = m_objects.size();
	else if(pos > oldPos)
		--pos;

	m_objects.insert(m_objects.begin() + pos, objectPtr);

	reorderedObjects.run();
}

PandaObject* ObjectsList::find(uint32_t objectIndex)
{
	auto iter = std::find_if(m_objects.cbegin(), m_objects.cend(), [objectIndex](const auto& object){
		return object->getIndex() == objectIndex;
	});

	if(iter != m_objects.end())
		return iter->get();

	return nullptr;
}

} // namespace panda


