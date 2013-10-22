#ifndef UPDATELOGGER_H
#define UPDATELOGGER_H

#include <chrono>
#include <QString>
#include <QVector>
#include <atomic>

namespace panda
{

namespace helper
{

enum EventType
{
	event_update,
	event_getValue,
	event_setDirty
};

// This is what will actually be stored
struct EventData
{
public:
	EventType m_type;
	std::chrono::high_resolution_clock::time_point m_start, m_end;
	QString m_name;
	quint32 m_index, m_level;
};

// To log an event, you only have to use this class
class ScopedEvent
{
public:
	ScopedEvent(EventType type, QString name, quint32 index);
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

	void clear();
	const UpdateEvents getEvents() const;

protected:
	UpdateLogger();
	friend class ScopedEvent;

	void addEvent(EventData event);

	UpdateEvents m_events;
	std::atomic_uint32_t m_level;
};

} // namespace helper

} // namespace panda

#endif // UPDATELOGGER_H
