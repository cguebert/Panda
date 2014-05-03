#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <atomic>
#include <functional>

#include <QMap>
#include <QVector>
#include <QThread>
#include <QRunnable>
#include <QMutex>
#include <QWaitCondition>

#include <boost/lockfree/queue.hpp>

namespace panda
{

class PandaDocument;
class PandaObject;
class DataNode;
class BaseData;
class SchedulerThread;

class Scheduler
{
public:
	Scheduler(PandaDocument* document);
	void init();
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
	void prepareThreads();

	friend class SchedulerThread;
	struct SchedulerTask;
	SchedulerTask* getTask(bool mainThread); // Get the next ready task
	void finishTask(SchedulerTask* task); // Call by a thread when a task is finished
	void readyTask(const SchedulerTask* task); // Add the task to the ready queue
	void testForEnd();

	QList<PandaObject*> expandObjectsList(QList<PandaObject*> objects);
	QVector<DataNode*> computeConnected(QVector<DataNode*> nodes) const; // Get the outputs of the nodes, sorted by distance
	QVector<DataNode*> computeConnected(DataNode* node) const;
	QVector<int> getTasks(QVector<DataNode*> nodes) const;
	void prepareLaterUpdate(BaseData* data);

	typedef std::function<void(PandaObject* object)> ObjectFunctor;
	void forEachObjectOutput(PandaObject* object, ObjectFunctor func);

	struct SchedulerTask
	{
		SchedulerTask() : restrictToMainThread(false), object(nullptr) { nbDirtyInputs = 0; }
		std::atomic_int nbDirtyInputs; // When this equal 0 (and is dirty), we can update the object
		int nbDirtyAtStart; // Value of nbDirtyInputs at the start of the timestep
		bool dirty; // First this has to become true to update the object
		bool dirtyAtStart; // Value of dirty at the start of the timestep
		bool restrictToMainThread; // For Objects that use OpenGL, update them only on the main thread
		PandaObject* object; // Object concerned by this task
		QVector<int> outputs; // Indices of other SchedulerTasks	
	};

	PandaDocument* m_document;
	QVector<DataNode*> m_setDirtyList; // At each step, all these nodes will always be dirty (connected to the mouse position or the animation time)
	QMap< BaseData*, QPair<QVector<DataNode*>, QVector<int> > > m_laterUpdatesMap; // For nodes that will get dirty later (like Buffer or Replicator)

	QVector<SchedulerTask> m_updateTasks;
	QMap<PandaObject*, int> m_objectsIndexMap;

	QVector<SchedulerThread*> m_updateThreads;

	boost::lockfree::queue<const SchedulerTask*> m_readyTasks, m_readyMainTasks;
	std::atomic_int m_nbReadyTasks;
};

//***************************************************************//

class SchedulerThread : public QRunnable
{
public:
	SchedulerThread(Scheduler* scheduler, int threadId);
	virtual void run();

	void close();
	void sleep();
	void wakeUp();

	int threadId() const;

protected:
	void idle();
	void doWork();

	Scheduler* m_scheduler;
	int m_threadId;
	bool m_mainThread;
	std::atomic_bool m_closing, m_canSleep, m_mustWakeUp;
};

inline void SchedulerThread::close()
{ m_closing = true; m_mustWakeUp = true; }

inline void SchedulerThread::sleep()
{ m_canSleep = true; m_mustWakeUp = false;  }

inline void SchedulerThread::wakeUp()
{ m_mustWakeUp = true; }

inline int SchedulerThread::threadId() const
{ return m_threadId; }

} // namespace panda

#endif // SCHEDULER_H
