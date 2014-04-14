#include <panda/Scheduler.h>
#include <panda/PandaDocument.h>

#include <QList>
#include <QSet>

namespace panda
{

Scheduler::Scheduler(PandaDocument* document)
	: m_document(document)
{
}

void Scheduler::init()
{
	// Create the dirty list
	// Use the 3 document datas we set at each step
	m_setDirtyList.clear();
	QList<DataNode*> openSet;
	QSet<DataNode*> closedSet;
	BaseData* timeData = m_document->getData("time");
	if(timeData)
		openSet.push_back(timeData);
	BaseData* mousePos = m_document->getData("mouse position");
	if(mousePos)
		openSet.push_back(mousePos);
	BaseData* mouseClick = m_document->getData("mouse click");
	if(mouseClick)
		openSet.push_back(mouseClick);

	// Get all the connected nodes
	while(!openSet.empty())
	{
		DataNode* node = openSet.front();
		openSet.pop_front();

		m_setDirtyList.push_back(node);

		if(closedSet.contains(node))
			continue;
		else
			closedSet.insert(node);

		for(auto output : node->getOutputs())
			openSet.push_back(output);
	}

	// Reverse the list (start from the node furthest from the document datas)
	std::reverse(m_setDirtyList.begin(), m_setDirtyList.end());
}

void Scheduler::setDirty()
{
	for(DataNode* node : m_setDirtyList)
		node->setDirtyValue();
}

void Scheduler::update()
{

}

} // namespace panda
