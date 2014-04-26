#ifndef UPDATELOGGER_H
#define UPDATELOGGER_H

#include <QString>
#include <QVector>
#include <QMap>

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
	int m_objectIndex, m_level, m_threadId;
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
	static int getThreadId();

	void startLog(PandaDocument* doc);
	void stopLog();

	void setNbThreads(int nbThreads);
	int getNbThreads() const;
	void setupThread(int id);

	const UpdateEvents getEvents(int id) const;
	const NodeStates getInitialNodeStates() const;

	static unsigned long long getTicksPerSec();
	static unsigned long long getTime();

protected:
	UpdateLogger();
	friend class ScopedEvent;
	friend class Scheduler;

	void addEvent(EventData event);
	int& logLevel(int threadId);

	QVector<UpdateEvents> m_events, m_prevEvents;
	int m_nbThreads;
	bool m_logging;
	NodeStates m_nodeStates, m_prevNodeStates;
	QVector<int> m_logLevelMap;
};

inline int& UpdateLogger::logLevel(int threadId)
{ return m_logLevelMap[threadId]; }

inline int UpdateLogger::getNbThreads() const
{ return m_nbThreads; }

} // namespace helper

} // namespace panda

#endif // UPDATELOGGER_H
