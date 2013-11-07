#ifndef UPDATELOGGER_H
#define UPDATELOGGER_H

#include <QString>
#include <QVector>
#include <QStack>
#include <atomic>

namespace panda
{

class PandaObject;
class BaseData;

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
	unsigned long long m_start, m_end;
	QString m_dataName, m_objectName;
	qint32 m_objectIndex, m_level;
};

// To log an event, you only have to use this class
class ScopedEvent
{
public:
	ScopedEvent(EventType type, const PandaObject* object);
	ScopedEvent(EventType type, const BaseData* data);
	~ScopedEvent();

private:
	EventData m_event;
};

// Container for all the events
class UpdateLogger
{
public:
	typedef QVector<EventData> UpdateEvents;

	static UpdateLogger* getInstance();

	void startLog();
	void stopLog();
	const UpdateEvents getEvents() const;

	static unsigned long long getTicksPerSec();
	static unsigned long long getTime();

protected:
	UpdateLogger();
	friend class ScopedEvent;

	void addEvent(EventData event);

	UpdateEvents m_events, m_prevEvents;
	int m_level;
	bool m_logging;
};

} // namespace helper

} // namespace panda

#endif // UPDATELOGGER_H
