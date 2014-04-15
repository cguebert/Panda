#include <panda/Scheduler.h>
#include <panda/PandaDocument.h>

#include <QList>
#include <QSet>

#include <iostream>

namespace panda
{

Scheduler::Scheduler(PandaDocument* document)
	: m_document(document)
{
}

// For debug purposes
QString getName(DataNode* node)
{
	BaseData* data = dynamic_cast<BaseData*>(node);
	if(data)
		return data->getOwner()->getName() + "/" + data->getName();
	PandaObject* object = dynamic_cast<PandaObject*>(node);
	if(object)
		return object->getName();

	return "";
}

void Scheduler::init()
{
	buildDirtyList();
	buildUpdateGraph();
}

void Scheduler::buildDirtyList()
{
	// Use the 3 document datas we set at each step
	m_setDirtyList.clear();
	QList<DataNode*> openSet, closedSet;
	BaseData* timeData = m_document->getData("time");
	if(timeData)
		openSet.push_back(timeData);
	BaseData* mousePos = m_document->getData("mouse position");
	if(mousePos)
		openSet.push_back(mousePos);
	BaseData* mouseClick = m_document->getData("mouse click");
	if(mouseClick)
		openSet.push_back(mouseClick);

	QMap<DataNode*, int> distanceMap;
	for(auto node : openSet)
		distanceMap[node] = 0;

	// Get all the connected nodes
	while(!openSet.empty())
	{
		DataNode* node = openSet.front();
		openSet.pop_front();

		int dist = distanceMap[node] + 1;

		closedSet.removeOne(node);
		closedSet.push_back(node);

		for(auto output : node->getOutputs())
		{
			distanceMap[output] = dist;
			openSet.removeOne(output);
			openSet.push_back(output);
		}
	}

	m_setDirtyList = closedSet.toVector();

	// Reverse the list (start from the node furthest from the document datas)
	std::reverse(m_setDirtyList.begin(), m_setDirtyList.end());
}

void Scheduler::buildUpdateGraph()
{

}

void Scheduler::setDirty()
{
	for(DataNode* node : m_setDirtyList)
		node->doSetDirty(); // Warning: this bypasses PandaObject::setDirtyValue
}

void Scheduler::update()
{

}

} // namespace panda
