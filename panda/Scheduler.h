#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QMap>
#include <QVector>

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
	struct SchedulerTask
	{
		int nbInputs;
		int nbDirtyInputs; // When this equal 0, we can update the object
		PandaObject* object;
	};

	PandaDocument* m_document;
	QVector<DataNode*> m_setDirtyList; // At each step, all these nodes will always be dirty (connected to the mouse position or the animation time)
};

} // namespace panda

#endif // SCHEDULER_H
