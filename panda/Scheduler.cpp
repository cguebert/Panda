#include <panda/Scheduler.h>
#include <panda/PandaDocument.h>
#include <panda/Renderer.h>

#include <QList>
#include <QQueue>
#include <QSet>
#include <QMutexLocker>
#include <QThreadPool>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

#include <iostream>

namespace panda
{

Scheduler::Scheduler(PandaDocument* document)
	: m_document(document)
	, m_readyTasks(256)
	, m_readyMainTasks(128)
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

void Scheduler::stop()
{
	if(!m_updateThreads.empty())
	{
		for(auto& thread : m_updateThreads)
			thread->close();

		delete m_updateThreads[0];

		m_updateThreads.clear();
	}
}

struct InputsListNode
{
	DataNode* node;
	int distance; // number of nodes between this node and the first node
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

QVector<DataNode*> Scheduler::computeConnected(QVector<DataNode*> nodes) const
{
	QQueue<DataNode*> openSet;
	QList<DataNode*> closedSet;
	QVector<InputsListNode> inputsList;
	QMap<DataNode*, int> indexMap;

	// Initialize the open list with the input nodes
	for(int i=0, nb=nodes.size(); i<nb; ++i)
	{
		DataNode* node = nodes[i];
		indexMap[node] = i;
		openSet.enqueue(node);
		InputsListNode iln;
		iln.node = node;
		iln.distance = 0;
		inputsList.push_back(iln);
	}

	// Get all the connected nodes
	while(!openSet.empty())
	{
		DataNode* node = openSet.dequeue();

		int parentId = indexMap[node];
		int dist = inputsList[parentId].distance + 1;

		closedSet.removeOne(node);
		closedSet.push_back(node);

		PandaObject* object = dynamic_cast<PandaObject*>(node);
		if(object && object->doesLaterUpdate())
			continue;

		for(auto output : node->getOutputs())
		{
			// We detect when there are loops in the graph
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
		}
	}

	QVector<DataNode*> result = closedSet.toVector();

	// Reverse the list (start from the node furthest from the input nodes)
	std::reverse(result.begin(), result.end());

	return result;
}

QVector<DataNode*> Scheduler::computeConnected(DataNode* node) const
{
	QVector<DataNode*> nodes;
	nodes.push_back(node);

	return computeConnected(nodes);
}

QVector<int> Scheduler::getTasks(QVector<DataNode*> nodes) const
{
	QVector<int> tasks;
	for(DataNode* node : nodes)
	{
		PandaObject* object = dynamic_cast<PandaObject*>(node);
		if(object)
			tasks.push_back(m_objectsIndexMap[object]);
	}

	return tasks;
}

void Scheduler::buildDirtyList()
{
	// Initialize the open list with the 3 document datas we modify at each time step
	QVector<DataNode*> nodes;
	const char* names[] = {"time", "mouse position", "mouse click"};
	for(auto name : names)
	{
		BaseData* data = m_document->getData(name);
		if(data)
			nodes.push_back(data);
	}

	m_setDirtyList = computeConnected(nodes);
}

void Scheduler::buildUpdateGraph()
{
	// Initialize the tasks list
	auto objects = m_document->getObjects();
	int nb = objects.size();
	m_updateTasks.clear();
	m_updateTasks.resize(nb);
	for(int i=0; i<nb; ++i)
	{
		PandaObject* object = objects[i];
		m_updateTasks[i].object = object;
		if(dynamic_cast<Layer*>(object))
			m_updateTasks[i].restrictToMainThread = true;
		m_objectsIndexMap[object] = i;
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
				if(m_objectsIndexMap.contains(object))
				{
					int id = m_objectsIndexMap[object];
					task.outputs.push_back(id);
				}
			}
			else if(data)
			{
				for(auto node : data->getOutputs())
				{
					BaseData* data2 = dynamic_cast<BaseData*>(node);
					if(data2 && data2->getOwner())
					{
						PandaObject* owner = data2->getOwner();
						if(m_objectsIndexMap.contains(owner))
						{
							int id = m_objectsIndexMap[owner];
							task.outputs.push_back(id);
						}
					}
				}
			}
		}
	}


	for(auto& task : m_updateTasks)
	{
		task.nbDirtyAtStart = 0;
		task.dirtyAtStart = false;
	}

	// Prepare the number of inputs that are dirty at the start of each time step
	for(auto node : m_setDirtyList)
	{
		PandaObject* parent = dynamic_cast<PandaObject*>(node);
		if(!parent || !m_objectsIndexMap.contains(parent))
			continue;

		m_updateTasks[m_objectsIndexMap[parent]].dirtyAtStart = true;

		for(auto output : parent->getOutputs())
		{
			PandaObject* object = dynamic_cast<PandaObject*>(output);
			BaseData* data = dynamic_cast<BaseData*>(output);
			if(object) // Some objects can be directly connected to others objects (Docks and Dockable for example)
			{
				if(m_objectsIndexMap.contains(object))
					++m_updateTasks[m_objectsIndexMap[object]].nbDirtyAtStart;
			}
			else if(data)
			{
				for(auto node : data->getOutputs())
				{
					BaseData* data2 = dynamic_cast<BaseData*>(node);
					if(data2 && data2->getOwner())
					{
						PandaObject* owner = data2->getOwner();
						if(m_objectsIndexMap.contains(owner))
							++m_updateTasks[m_objectsIndexMap[owner]].nbDirtyAtStart;
					}
				}
			}
		}
	}

	for(auto& task : m_updateTasks)
	{
		if(task.nbDirtyAtStart > 0)
			task.dirtyAtStart = true;
	}

	// Take care of laterUpdate objects
	m_laterUpdatesMap.clear();
	for(int i=0; i<nb; ++i)
	{
		PandaObject* object = objects[i];
		if(object->doesLaterUpdate())
		{
			for(DataNode* node : object->getOutputs())
				prepareLaterUpdate(node);
		}
	}
/*
	// Debug
	for(int i=0; i<nb; ++i)
	{
		const auto& task = m_updateTasks[i];
		std::cout << i << " " << task.object->getName().toStdString() << std::endl;
		std::cout << "\tDirtyInputs: " << task.nbDirtyAtStart << "\t" << (task.dirtyAtStart?"dirtyAtStart":"") << std::endl;
		std::cout << "\tOutputs: ";
		for(auto output : task.outputs)
			std::cout << output << " ";
		std::cout << std::endl;
	}*/
}

void Scheduler::prepareThreads()
{
	int nbThreads = qMax(1, QThread::idealThreadCount() / 2);

	m_updateThreads.push_back(new SchedulerThread(this, 0));
	for(int i=1; i<nbThreads; ++i)
	{
		SchedulerThread* thread = new SchedulerThread(this, i);

		if(QThreadPool::globalInstance()->tryStart(thread))
			m_updateThreads.push_back(thread);
		else
			delete thread;
	}
	
#ifdef PANDA_LOG_EVENTS
	helper::UpdateLogger::getInstance()->setNbThreads(nbThreads);
	helper::UpdateLogger::getInstance()->setupThread(0);
#endif
}

void Scheduler::setDirty()
{
#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log(helper::event_update, -1, "Scheduler/SetDirty");
#endif
	for(DataNode* node : m_setDirtyList)
		node->doSetDirty(); // Warning: this bypasses PandaObject::setDirtyValue

	for(auto& task : m_updateTasks)
	{
		task.nbDirtyInputs = task.nbDirtyAtStart;
		task.dirty = task.dirtyAtStart;
	}
}

void Scheduler::update()
{
#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log(helper::event_update, -1, "Scheduler");
#endif

	m_nbReadyTasks = 0;

	for(auto& task : m_updateTasks)
	{
		if(task.dirty && !task.nbDirtyInputs)
			readyTask(&task);
	}

	for(auto& thread : m_updateThreads)
		thread->wakeUp();

	if(!m_updateThreads.empty())
		m_updateThreads[0]->run();
}

void Scheduler::prepareLaterUpdate(DataNode* node)
{
	auto connected = computeConnected(node);
	m_laterUpdatesMap[node] = qMakePair(connected, getTasks(connected));
}

void Scheduler::setNodeDirty(DataNode* dirtyNode)
{
	if(!m_laterUpdatesMap.contains(dirtyNode))
		prepareLaterUpdate(dirtyNode);

	auto pair = m_laterUpdatesMap[dirtyNode];
	for(auto* node : pair.first)
		node->doSetDirty();
	for(auto taskId : pair.second)
	{
		auto& task = m_updateTasks[taskId];
		++task.nbDirtyInputs;
		task.dirty = true;
	}
}

void Scheduler::setNodeReady(DataNode* node)
{
	Q_UNUSED(node)
}

Scheduler::SchedulerTask* Scheduler::getTask(bool mainThread)
{
	Scheduler::SchedulerTask* task;
	if(mainThread)
	{
		if(m_readyMainTasks.pop(task))
			return task;
	}

	if(m_readyTasks.pop(task))
		return task;

	return nullptr;
}

void Scheduler::finishTask(SchedulerTask* task)
{
	task->dirty = false;
	for(auto output : task->outputs)
	{
		auto& outputTask = m_updateTasks[output];
		if(outputTask.dirty && (--outputTask.nbDirtyInputs) <= 0)
			readyTask(&outputTask);
	}
	m_nbReadyTasks--;
}

void Scheduler::readyTask(const SchedulerTask* task)
{
	m_nbReadyTasks++;

	if(task->restrictToMainThread)
		m_readyMainTasks.push(task);
	else
		m_readyTasks.push(task);
}

void Scheduler::testForEnd()
{
	if (!m_nbReadyTasks)
	{
		for(auto& thread : m_updateThreads)
			thread->sleep();
	}
}

//***************************************************************//

SchedulerThread::SchedulerThread(Scheduler* scheduler, int threadId)
	: m_scheduler(scheduler)
	, m_threadId(threadId)
	, m_mainThread(threadId == 0)
{
	m_closing = false;
	m_canSleep = false;
	m_mustWakeUp = false;
}

void SchedulerThread::run()
{
#ifdef PANDA_LOG_EVENTS
	helper::UpdateLogger::getInstance()->setupThread(m_threadId);
#endif

	while(!m_closing)
	{
		if(!m_mainThread)
			idle();

		if(m_closing)
			break;

		while(true)
		{
			doWork();

			if (m_mainThread)
				m_scheduler->testForEnd();

			if(m_canSleep)
			{
				m_canSleep = false;
				if(m_mainThread)
					return;
				break;
			}

			if (m_closing)
				return;
		}
	}
}

void SchedulerThread::idle()
{
	while(!m_mustWakeUp)
		QThread::yieldCurrentThread();

	m_mustWakeUp = false;
}

void SchedulerThread::doWork()
{
	while(Scheduler::SchedulerTask* task = m_scheduler->getTask(m_mainThread))
	{
		task->object->updateIfDirty();
		m_scheduler->finishTask(task);
	}
}

} // namespace panda
