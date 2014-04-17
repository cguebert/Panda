#include <panda/Scheduler.h>
#include <panda/PandaDocument.h>
#include <panda/Renderer.h>

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
	QList<DataNode*> openSet, closedSet;
	QVector<InputsListNode> inputsList;
	QMap<DataNode*, int> indexMap;

	// Initialize the open list with the 3 document datas we modify at each time step
	const char* names[] = {"time", "mouse position", "mouse click"};
	for(auto name : names)
	{
		BaseData* data = m_document->getData(name);
		if(data)
		{
			openSet.push_back(data);
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
		DataNode* node = openSet.front();
		openSet.pop_front();

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
				openSet.push_back(output);

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

	QMap<PandaObject*, int> objectsIndexMap;
	int nb = m_updateList.size();
	m_updateTasks.resize(nb);
	for(int i=0; i<nb; ++i)
	{
		PandaObject* object = m_updateList[i];
		m_updateTasks[i].object = object;
		objectsIndexMap[object] = i;
	}

	// TODO: compute outputs objects of each object
}

void Scheduler::setDirty()
{
	for(DataNode* node : m_setDirtyList)
		node->doSetDirty(); // Warning: this bypasses PandaObject::setDirtyValue
}

void Scheduler::update()
{
	for(auto& task : m_updateTasks)
		task.nbDirtyInputs = task.nbInputs;

	for(auto object : m_updateList)
		object->updateIfDirty();
}

} // namespace panda
