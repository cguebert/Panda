#include <panda/PandaDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/NodeUpdater.h>
#include <panda/helper/algorithm.h>

#include <deque>

namespace
{

void merge(std::vector<panda::PandaObject*>& mainList, 
	const std::vector<panda::PandaObject*>& addList, 
	std::set<panda::PandaObject*>& closedSet)
{
	for (auto obj : addList)
	{
		if (closedSet.count(obj))
			continue;

		mainList.push_back(obj);
		closedSet.insert(obj);
	}
}

}

namespace panda
{

NodeUpdater::NodeUpdater(PandaDocument& document)
	: m_document(document)
{
	auto& signals = m_document.getSignals();

	m_observer.get(signals.addedObject).connect<NodeUpdater, &NodeUpdater::clear>(this);
	m_observer.get(signals.removedObject).connect<NodeUpdater, &NodeUpdater::clear>(this);
	m_observer.get(signals.modifiedObject).connect<NodeUpdater, &NodeUpdater::clear>(this);
}

void NodeUpdater::updateObject(PandaObject& object)
{
	if (!object.isDirty())
		return;

	for (auto obj : getUpdateList(object))
		obj->updateIfDirty();
}

void NodeUpdater::clear(PandaObject*)
{
	m_updateMap.clear();
}

const NodeUpdater::ObjectsList& NodeUpdater::getUpdateList(PandaObject& object)
{
	auto objectPtr = &object;
	auto& updateList = m_updateMap[objectPtr];
	if (!updateList.empty())
		return updateList;
	
	ObjectsSet closed;
	closed.insert(objectPtr);
	graph::forEachObjectInput(objectPtr, [this, &updateList, &closed](PandaObject* obj) {
		merge(updateList, getUpdateList(*obj), closed);
	});

	updateList.push_back(objectPtr);
	return updateList;
}

} // namespace panda
