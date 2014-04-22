#ifndef UPDATELOGGER_H
#define UPDATELOGGER_H

#include <QString>
#include <QVector>
#include <QMap>
#include <atomic>

namespace panda
{

class PandaObject;
class PandaDocument;
class BaseData;
class DataNode;
class Scheduler;

namespace helper
{

enum EventType
{
	event_update,
	event_getValue,
	event_render,
	event_copyValue,
	event_setDirty
};

// This is what will actually be stored
struct EventData
{
public:
	EventType m_type;
	unsigned long long m_startTime, m_endTime;
	QString m_dataName, m_objectName;
	qint32 m_objectIndex, m_level;
	bool m_dirtyStart, m_dirtyEnd;
	const DataNode* m_node;
};

// To log an event, you only have to use this class
class ScopedEvent
{
public:
	ScopedEvent(EventType type, const PandaObject* object);
	ScopedEvent(EventType type, const BaseData* data);
	ScopedEvent(EventType type, int index, QString name);
	~ScopedEvent();

private:
	EventData m_event;
};

// Container for all the events
class UpdateLogger
{
public:
	typedef QVector<EventData> UpdateEvents;
	typedef QMap<const DataNode*, bool> NodeStates;

	static UpdateLogger* getInstance();

	void startLog(PandaDocument* doc);
	void stopLog();

	const int nbThreads() const;
	const UpdateEvents getEvents(int id) const;
	const NodeStates getInitialNodeStates() const;

	static unsigned long long getTicksPerSec();
	static unsigned long long getTime();

protected:
	UpdateLogger();
	friend class ScopedEvent;
	friend class Scheduler;

	void setNbThreads(int nbThreads);
	void setThreadId(int id);
	void addEvent(EventData event);

	QVector<UpdateEvents> m_events, m_prevEvents;
	int m_level, m_nbThreads;
	bool m_logging;
	NodeStates m_nodeStates, m_prevNodeStates;
};

} // namespace helper

} // namespace panda

#endif // UPDATELOGGER_H
