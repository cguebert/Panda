#include <panda/Scheduler.h>
#include <panda/PandaDocument.h>
#include <panda/Renderer.h>

#include <QList>
#include <QQueue>
#include <QSet>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

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

	prepareThreads();
}

struct InputsListNode
{
	DataNode* node;
	int distance; // number of nodes between this node and the document
	QVector<int> parentsId;
};

bool containsInput(const QVector<InputsListNode>& inputsList, int currentNodeId, const DataNode* nextNode)
{
	if(currentNodeId < 0 || currentNodeId >= inputsList.size())
		return false;

	QList<int> openSet;
	openSet.push_back(currentNodeId);
	while(!openSet.empty())
	{
		const InputsListNode& inputNode = inputsList[openSet.front()];
		openSet.pop_front();
		if(nextNode == inputNode.node)
			return true;

		for(int parentId : inputNode.parentsId)
			openSet.push_back(parentId);
	}

	return false;
}

void Scheduler::buildDirtyList()
{
	m_setDirtyList.clear();
	QQueue<DataNode*> openSet;
	QList<DataNode*> closedSet;
	QVector<InputsListNode> inputsList;
	QMap<DataNode*, int> indexMap;

	// Initialize the open list with the 3 document datas we modify at each time step
	const char* names[] = {"time", "mouse position", "mouse click"};
	for(auto name : names)
	{
		BaseData* data = m_document->getData(name);
		if(data)
		{
			openSet.enqueue(data);
			indexMap[data] = inputsList.size();
			InputsListNode node;
			node.node = data;
			node.distance = 0;
			inputsList.push_back(node);
		}
	}

	// Get all the connected nodes
	while(!openSet.empty())
	{
		DataNode* node = openSet.dequeue();

		int parentId = indexMap[node];
		int dist = inputsList[parentId].distance + 1;

		closedSet.removeOne(node);
		closedSet.push_back(node);

		for(auto output : node->getOutputs())
		{
			if(!containsInput(inputsList, parentId, output))
			{
				// Move the node on the back of the open list
				openSet.removeOne(output);
				openSet.enqueue(output);

				// If node was already treated, update it
				if(indexMap.contains(output))
				{
					InputsListNode& node = inputsList[indexMap[output]];
					node.distance = dist;
					if(!node.parentsId.contains(parentId))
						node.parentsId.push_back(parentId);
				}
				else // Create it
				{
					int id = inputsList.size();
					indexMap[output] = id;
					InputsListNode node;
					node.node = output;
					node.distance = dist;
					node.parentsId.push_back(parentId);
					inputsList.push_back(node);
				}
			}
			else
			{
			//	std::cout << "Detected a loop in the graph starting at: " << getName(output).toStdString() << std::endl;
			}
		}
	}

	m_setDirtyList = closedSet.toVector();

	// Reverse the list (start from the node furthest from the document datas)
	std::reverse(m_setDirtyList.begin(), m_setDirtyList.end());
}

void Scheduler::buildUpdateGraph()
{
	// We copy the dirty list computed previously, reverse it and extract only objects
	auto tempNodeList = m_setDirtyList;
	std::reverse(tempNodeList.begin(), tempNodeList.end());
	m_updateList.clear();
	for(auto node : tempNodeList)
	{
		if(dynamic_cast<PandaDocument*>(node)) continue;

		PandaObject* object = dynamic_cast<PandaObject*>(node);
		if(object)
			m_updateList.push_back(object);
	}

	// Initialize the tasks list
	QMap<PandaObject*, int> objectsIndexMap;
	int nb = m_updateList.size();
	m_updateTasks.clear();
	m_updateTasks.resize(nb);
	for(int i=0; i<nb; ++i)
	{
		PandaObject* object = m_updateList[i];
		m_updateTasks[i].object = object;
		objectsIndexMap[object] = i;
	}

	// Compute outputs of each object
	for(auto& task : m_updateTasks)
	{
		for(auto output : task.object->getOutputs())
		{
			PandaObject* object = dynamic_cast<PandaObject*>(output);
			BaseData* data = dynamic_cast<BaseData*>(output);
			if(object) // Some objects can be directly connected to others objects (Docks and Dockable for example)
			{
				if(objectsIndexMap.contains(object))
				{
					int id = objectsIndexMap[object];
					task.outputs.push_back(id);
					++m_updateTasks[id].nbInputs;
				}
			}
			else if(data)
			{
				for(auto node : data->getOutputs())
				{
					BaseData* data2 = dynamic_cast<BaseData*>(node);
					if(data2 && data2->getOwner())
					{
						PandaObject* object2 = data2->getOwner();
						if(objectsIndexMap.contains(object2))
						{
							int id = objectsIndexMap[object2];
							task.outputs.push_back(id);
							++m_updateTasks[id].nbInputs;
						}
					}
				}
			}
		}
	}
/*
	// Debug
	for(int i=0; i<nb; ++i)
	{
		const auto& task = m_updateTasks[i];
		std::cout << i << " " << task.object->getName().toStdString() << std::endl;
		for(auto output : task.outputs)
			std::cout << output << " ";
		std::cout << std::endl;
	}*/
}

void Scheduler::prepareThreads()
{
	int nbThreads = qMax(1, QThread::idealThreadCount() / 2);

#ifdef PANDA_LOG_EVENTS
	helper::UpdateLogger::getInstance()->setNbThreads(nbThreads);
	helper::UpdateLogger::getInstance()->setThreadId(0);
#endif
}

void Scheduler::setDirty()
{
#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log(helper::event_update, -1, "Scheduler/SetDirty");
#endif
	for(DataNode* node : m_setDirtyList)
		node->doSetDirty(); // Warning: this bypasses PandaObject::setDirtyValue
}

void Scheduler::update()
{
#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log(helper::event_update, -1, "Scheduler");
#endif

	for(auto& task : m_updateTasks)
		task.nbDirtyInputs = task.nbInputs;

	for(auto object : m_updateList)
		object->updateIfDirty();
}

} // namespace panda
