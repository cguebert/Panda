#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QMap>
#include <QVector>
#include <QThread>
#include <atomic>

namespace panda
{

class PandaDocument;
class PandaObject;
class DataNode;

class Scheduler
{
public:
	Scheduler(PandaDocument* document);
	void init();
	void setDirty();
	void update();

protected:
	void buildDirtyList();
	void buildUpdateGraph();
	void prepareThreads();

	struct SchedulerTask
	{
		SchedulerTask() : nbInputs(0), object(nullptr) { nbDirtyInputs = 0; }
		int nbInputs;
		std::atomic_int nbDirtyInputs; // When this equal 0, we can update the object
		PandaObject* object;
		QVector<int> outputs; // Indices of other SchedulerTasks
	};

	PandaDocument* m_document;
	QVector<DataNode*> m_setDirtyList; // At each step, all these nodes will always be dirty (connected to the mouse position or the animation time)

	QVector<PandaObject*> m_updateList; // TEST: works only for monothread for now
	QVector<SchedulerTask> m_updateTasks;
};

} // namespace panda

#endif // SCHEDULER_H
