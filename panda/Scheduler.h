#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <atomic>

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
class SchedulerThread;

class Scheduler
{
public:
	Scheduler(PandaDocument* document);
	void init();
	void stop();

	void setDirty();
	void update();

protected:
	void buildDirtyList();
	void buildUpdateGraph();
	void prepareThreads();

	friend class SchedulerThread;
	struct SchedulerTask;
	const SchedulerTask* getTask(bool mainThread); // Get the next ready task
	void finishTask(const SchedulerTask *task); // Call by a thread when a task is finished
	void readyTask(const SchedulerTask *task); // Add the task to the ready queue
	void testForEnd();

	struct SchedulerTask
	{
		SchedulerTask() : nbInputs(0), restrictToMainThread(false), object(nullptr) { nbDirtyInputs = 0; }
		int index;
		int nbInputs;
		std::atomic_int nbDirtyInputs; // When this equal 0, we can update the object
		bool restrictToMainThread; // For Objects that use OpenGL, update them only on the main thread
		PandaObject* object;
		QVector<int> outputs; // Indices of other SchedulerTasks
	};

	PandaDocument* m_document;
	QVector<DataNode*> m_setDirtyList; // At each step, all these nodes will always be dirty (connected to the mouse position or the animation time)

	QVector<PandaObject*> m_updateList; // TEST: works only for monothread for now
	QVector<SchedulerTask> m_updateTasks;

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
