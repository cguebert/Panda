#include <panda/PandaDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/NodeUpdater.h>
#include <panda/helper/algorithm.h>

#include <deque>
#include <set>

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

	auto& updateList = m_updateMap[&object];
	if (updateList.empty())
		updateList = buildUpdateList(object);

	for (auto obj : updateList)
		obj->updateIfDirty();
}

void NodeUpdater::clear(PandaObject*)
{
	m_updateMap.clear();
}

NodeUpdater::ObjectsList NodeUpdater::buildUpdateList(PandaObject& object)
{
	std::set<PandaObject*> closedSet;
	std::deque<PandaObject*> openSet;
	std::vector<PandaObject*> updateList;

	auto func = [&closedSet, &openSet, &updateList](PandaObject* in) {
		if (closedSet.count(in)) // Move this object to the end, as it needs to be updated sooner than we first thought
		{
			if (updateList.back() != in)
			{
				auto it = std::find(updateList.begin(), updateList.end(), in);
				if (it != updateList.end())
					std::rotate(it, std::next(it), updateList.end());
			}
		}
		else
		{
			openSet.push_back(in);
			updateList.push_back(in);
			closedSet.insert(in);
		}
	};

	auto objectPtr = &object;
	updateList.push_back(objectPtr);
	closedSet.insert(objectPtr);
	graph::forEachObjectInput(objectPtr, func);

	while (!openSet.empty())
	{
		auto obj = openSet.front();
		openSet.pop_front();

		graph::forEachObjectInput(obj, func);
	}

	std::reverse(updateList.begin(), updateList.end());
	return updateList;
}

} // namespace panda
