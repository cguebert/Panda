#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <panda/core.h>

#ifdef _MSC_VER
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#endif

#include <atomic>

#ifdef _MSC_VER
#undef _ENABLE_ATOMIC_ALIGNMENT_FIX
#endif

#include <functional>
#include <memory>
#include <map>
#include <vector>

#include <boost/lockfree/queue.hpp>

namespace panda
{

class PandaDocument;
class PandaObject;
class DataNode;
class BaseData;
class SchedulerThread;

class PANDA_CORE_API Scheduler
{
public:
	Scheduler(PandaDocument* document);
	void init(int nbThreads = -1); // If -1, use half of hardware concurrency
	void stop();

	void setDirty();
	void update();
	void waitForOtherTasks(bool mainThread); // Work until there is only 1 task running (useful for the Replicator)

	void setDataDirty(BaseData* data); // Set the outputs to dirty before setting the value (so it doesn't propagate)
	void setDataReady(BaseData* data); // Launch the tasks connected to this node

protected:
	void buildDirtyList();
	void buildUpdateGraph();
	void computeStartValues();
	void prepareThreads(int nbThreads = -1);

	friend class SchedulerThread;
	struct SchedulerTask;
	SchedulerTask* getTask(bool mainThread); // Get the next ready task
	void finishTask(SchedulerTask* task); // Call by a thread when a task is finished
	void readyTask(const SchedulerTask* task); // Add the task to the ready queue
	void testForEnd();

	std::vector<DataNode*> computeConnected(const std::vector<DataNode*>& nodes) const; // Get the outputs of the nodes, sorted by distance
	std::vector<DataNode*> computeConnected(DataNode* node) const;
	std::vector<int> getTasks(const std::vector<DataNode*>& nodes) const;
	void prepareLaterUpdate(BaseData* data);

	struct SchedulerTask
	{
		SchedulerTask() { nbDirtyInputs = 0; }
		SchedulerTask(const SchedulerTask&);
		std::atomic_int nbDirtyInputs; // When this equal 0 (and is dirty), we can update the object
		int nbDirtyAtStart = 0; // Value of nbDirtyInputs at the start of the timestep
		bool dirty = false; // First this has to become true to update the object
		bool dirtyAtStart = false; // Value of dirty at the start of the timestep
		bool restrictToMainThread = false; // For Objects that use OpenGL, update them only on the main thread
		PandaObject* object = nullptr; // Object concerned by this task
		std::vector<int> outputs; // Indices of other SchedulerTasks	
	};

	PandaDocument* m_document;
	std::vector<DataNode*> m_setDirtyList; // At each step, all these nodes will always be dirty (connected to the mouse position or the animation time)
	std::map< BaseData*, std::pair<std::vector<DataNode*>, std::vector<int> > > m_laterUpdatesMap; // For nodes that will get dirty later (like Buffer or Replicator)

	std::vector<SchedulerTask> m_updateTasks;
	std::map<PandaObject*, int> m_objectsIndexMap;

	std::vector<std::shared_ptr<SchedulerThread>> m_updateThreads;

	boost::lockfree::queue<const SchedulerTask*> m_readyTasks, m_readyMainTasks;
	std::atomic_int m_nbReadyTasks;
};

} // namespace panda

#endif // SCHEDULER_H
