#include <panda/PandaDocument.h>
#include <panda/helper/UpdateLogger.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/Scheduler.h>
#include <panda/helper/algorithm.h>

#include <deque>
#include <thread>

namespace panda
{

class SchedulerThread
{
public:
	SchedulerThread(Scheduler* scheduler, int threadId);
	void run();

	void close();
	void sleep();
	void wakeUp();

	using ThreadPtr = std::shared_ptr<std::thread>;
	void setThread(ThreadPtr ptr);
	void joinThread();

	int threadId() const;

protected:
	void idle();
	void doWork();

	Scheduler* m_scheduler;
	ThreadPtr m_thread;
	int m_threadId;
	bool m_mainThread;
	std::atomic_bool m_closing, m_canSleep, m_mustWakeUp;
};

inline void SchedulerThread::close()
{
	m_closing = true; m_mustWakeUp = true; m_canSleep = false;
}

inline void SchedulerThread::sleep()
{
	m_canSleep = true; m_mustWakeUp = false;
}

inline void SchedulerThread::wakeUp()
{
	m_mustWakeUp = true;
}

inline void SchedulerThread::setThread(ThreadPtr ptr)
{
	m_thread = ptr;
}

inline void SchedulerThread::joinThread()
{
	if (m_thread && m_thread->joinable())
		m_thread->join();
}

inline int SchedulerThread::threadId() const
{
	return m_threadId;
}

//****************************************************************************//

// It is not used currently as we build the vector all in one go
//  but we must at least define this constructor as we have an atomic member (non copyable, non movable)
Scheduler::SchedulerTask::SchedulerTask(const SchedulerTask& rhs)
{
	nbDirtyInputs.store(rhs.nbDirtyInputs);
	nbDirtyAtStart = rhs.nbDirtyAtStart;
	dirty = rhs.dirty;
	dirtyAtStart = rhs.dirtyAtStart;
	restrictToMainThread = rhs.restrictToMainThread;
	object = rhs.object;
	outputs = rhs.outputs;
}

//****************************************************************************//

Scheduler::Scheduler(PandaDocument* document)
	: m_document(document)
	, m_readyTasks(256)
	, m_readyMainTasks(128)
{
}

void Scheduler::init(int nbThreads)
{
	buildDirtyList();
	buildUpdateGraph();

	prepareThreads(nbThreads);
}

void Scheduler::stop()
{
	if(!m_updateThreads.empty())
	{
		for(auto& thread : m_updateThreads)
			thread->close();

		for (auto& thread : m_updateThreads)
			thread->joinThread();

		m_updateThreads.clear();
	}
}

struct InputsListNode
{
	DataNode* node;
	int distance; // number of nodes between this node and the first node
	std::vector<int> parentsId;
};

bool containsInput(const std::vector<InputsListNode>& inputsList, int currentNodeId, const DataNode* nextNode)
{
	if(currentNodeId < 0 || currentNodeId >= static_cast<int>(inputsList.size()))
		return false;

	std::deque<int> openSet;
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

std::vector<DataNode*> Scheduler::computeConnected(const std::vector<DataNode*>& nodes) const
{
	std::deque<DataNode*> openSet;
	std::vector<DataNode*> closedSet;
	std::vector<InputsListNode> inputsList;
	std::map<DataNode*, int> indexMap;

	// Initialize the open list with the input nodes
	for(int i=0, nb=nodes.size(); i<nb; ++i)
	{
		DataNode* node = nodes[i];
		indexMap[node] = i;
		openSet.push_back(node);
		InputsListNode iln;
		iln.node = node;
		iln.distance = 0;
		inputsList.push_back(iln);
	}

	// Get all the connected nodes
	while(!openSet.empty())
	{
		DataNode* node = openSet.front();
		openSet.pop_front();

		int parentId = indexMap[node];
		int dist = inputsList[parentId].distance + 1;

		helper::removeOne(closedSet, node);
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
				helper::removeOne(openSet, output);
				openSet.push_back(output);

				// If node was already treated, update it
				if(indexMap.count(output))
				{
					InputsListNode& node = inputsList[indexMap[output]];
					node.distance = dist;
					if(!helper::contains(node.parentsId, parentId))
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
//			else
//				std::cout << "Ignoring node " << getName(output).toStdString() << std::endl;
		}
	}

	std::vector<DataNode*> result(closedSet.begin(), closedSet.end());

	// Reverse the list (start from the node furthest from the input nodes)
	std::reverse(result.begin(), result.end());

	return result;
}

std::vector<DataNode*> Scheduler::computeConnected(DataNode* node) const
{
	std::vector<DataNode*> nodes;
	nodes.push_back(node);

	return computeConnected(nodes);
}

std::vector<int> Scheduler::getTasks(const std::vector<DataNode*>& nodes) const
{
	std::vector<int> tasks;
	for(DataNode* node : nodes)
	{
		PandaObject* object = dynamic_cast<PandaObject*>(node);
		if(object)
			tasks.push_back(m_objectsIndexMap.at(object));
	}

	return tasks;
}

void Scheduler::buildDirtyList()
{
	// Initialize the open list with the 3 document datas we modify at each time step
	std::vector<DataNode*> nodes;
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
	std::vector<PandaObject*> objects;
	for(auto& object : m_document->getObjects())
		objects.push_back(object.get());
	objects = graph::expandObjectsList(objects);
	int nb = objects.size();
	m_updateTasks.clear();
	m_updateTasks.resize(nb);
	for(int i=0; i<nb; ++i)
	{
		PandaObject* object = objects[i];
		m_updateTasks[i].object = object;
		if(object->updateOnMainThread())
			m_updateTasks[i].restrictToMainThread = true;
		m_objectsIndexMap[object] = i;
	}

	// Compute outputs of each object
	for(auto& task : m_updateTasks)
	{
		graph::forEachObjectOutput(task.object, [this, &task](PandaObject* object){
			if(m_objectsIndexMap.count(object))
				task.outputs.push_back(m_objectsIndexMap[object]);
		});
	}

	// Compute dirty flag and number of dirty inputs for objects at the start of the timestep
	computeStartValues();

	// Take care of laterUpdate objects
	m_laterUpdatesMap.clear();
	for(int i=0; i<nb; ++i)
	{
		PandaObject* object = objects[i];
		if(object->doesLaterUpdate())
		{
			for(BaseData* data : object->getOutputDatas())
				prepareLaterUpdate(data);
		}
	}
}

void Scheduler::computeStartValues()
{
	for(auto& task : m_updateTasks)
	{
		task.nbDirtyAtStart = 0;
		task.dirtyAtStart = false;
	}

	// Prepare the number of inputs that are dirty at the start of each time step
	for(auto node : m_setDirtyList)
	{
		PandaObject* object = dynamic_cast<PandaObject*>(node);
		if(!object || !m_objectsIndexMap.count(object) || object->doesLaterUpdate())
			continue;

		m_updateTasks[m_objectsIndexMap[object]].dirtyAtStart = true;

		graph::forEachObjectOutput(object, [this](PandaObject* object){
			if (m_objectsIndexMap.count(object))
				++m_updateTasks[m_objectsIndexMap[object]].nbDirtyAtStart;
		});
	}

	for(auto& task : m_updateTasks)
	{
		if(task.nbDirtyAtStart > 0)
			task.dirtyAtStart = true;
	}
}

void Scheduler::prepareThreads(int nbThreads)
{
	if(nbThreads < 0)
		nbThreads = std::max(1u, std::thread::hardware_concurrency() / 2);

	m_updateThreads.push_back(std::make_shared<SchedulerThread>(this, 0));
	for(int i=1; i<nbThreads; ++i)
	{
		auto st = std::make_shared<SchedulerThread>(this, i);
		m_updateThreads.push_back(st);
		auto thread = std::make_shared<std::thread>(&SchedulerThread::run, st.get());
		st->setThread(thread);
	}
	
	helper::UpdateLogger::getInstance()->setNbThreads(nbThreads);
	helper::UpdateLogger::getInstance()->setupThread(0);
}

void Scheduler::setDirty()
{
	{
		helper::ScopedEvent log("Scheduler/setDirty");

		for(DataNode* node : m_setDirtyList)
			node->doSetDirty(); // Warning: this bypasses PandaObject::setDirtyValue

		for(auto& task : m_updateTasks)
		{
			task.nbDirtyInputs = task.nbDirtyAtStart;
			task.dirty = task.dirtyAtStart;
		}
	}
	helper::UpdateLogger::getInstance()->updateDirtyStates();
}

void Scheduler::update()
{
	helper::ScopedEvent log("Scheduler/update");
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

void Scheduler::waitForOtherTasks(bool mainThread)
{
	while(m_nbReadyTasks > 1)
	{
		while(SchedulerTask* task = getTask(mainThread))
		{
			task->dirty = false;
			task->object->updateIfDirty();
			finishTask(task);
		}
	}
}

void Scheduler::prepareLaterUpdate(BaseData* data)
{
	auto connected = computeConnected(data);

	std::vector<int> outputsTasks;
	for(auto output : data->getOutputs())
	{
		PandaObject* object = dynamic_cast<PandaObject*>(output);
		BaseData* data = dynamic_cast<BaseData*>(output);
		if(object)
		{
			if (m_objectsIndexMap.count(object))
				outputsTasks.push_back(m_objectsIndexMap[object]);
		}
		else if(data)
		{
			PandaObject* owner = data->getOwner();
			if (owner && m_objectsIndexMap.count(owner))
				outputsTasks.push_back(m_objectsIndexMap[owner]);
		}
	}

	m_laterUpdatesMap[data] = std::make_pair(connected, outputsTasks);
}

void Scheduler::setDataDirty(BaseData* dirtyData)
{
	helper::ScopedEvent log("Scheduler/setDataDirty");

	if (!m_laterUpdatesMap.count(dirtyData))
		prepareLaterUpdate(dirtyData);

	const auto& pair = m_laterUpdatesMap[dirtyData];
	for(auto node : pair.first)
		node->doSetDirty();

	// For outputs tasks, we still have to do some recursion
	std::deque<int> openSet;
	for(auto taskId : pair.second)
		openSet.push_back(taskId);

	while(!openSet.empty())
	{
		int taskId = openSet.front();
		openSet.pop_front();
		auto& task = m_updateTasks[taskId];
		++task.nbDirtyInputs;
		if(!task.dirty)
		{
			task.dirty = true;
			for(auto outputs : task.outputs)
				openSet.push_back(outputs);
		}
	}
}

void Scheduler::setDataReady(BaseData* data)
{
	helper::ScopedEvent log("Scheduler/setDataReady");

	if(!m_laterUpdatesMap.count(data))
		return;

	for(auto taskId : m_laterUpdatesMap[data].second)
	{
		auto& task = m_updateTasks[taskId];
		if(task.dirty && !(--task.nbDirtyInputs))
			readyTask(&task);
	}
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
	for(auto output : task->outputs)
	{
		auto& outputTask = m_updateTasks[output];
		if(outputTask.dirty && !(--outputTask.nbDirtyInputs))
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

//****************************************************************************//

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
	helper::UpdateLogger::getInstance()->setupThread(m_threadId);

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
	while (!m_mustWakeUp)
		std::this_thread::yield();

	m_mustWakeUp = false;
}

void SchedulerThread::doWork()
{
	while(Scheduler::SchedulerTask* task = m_scheduler->getTask(m_mainThread))
	{
		task->dirty = false;
		task->object->updateIfDirty();
		m_scheduler->finishTask(task);
	}
}

} // namespace panda
