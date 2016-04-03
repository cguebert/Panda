#ifndef UPDATELOGGER_H
#define UPDATELOGGER_H

#include <panda/core.h>

#include <map>
#include <string>
#include <vector>

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
	event_update,	// update of a DataNode
	event_getValue, // getValue of a Data
	event_render,	// render of a Renderer
	event_copyValue,// When it is necessary to copy a value during getValue
	event_setDirty, // setDirty of a DataNode
	event_custom	// For any other event we want to time, that does not have to be connected to a DataNode
};

// This is what will actually be stored
struct EventData
{
public:
	EventType m_type;
	long long m_startTime, m_endTime;
	std::string m_text;
	int m_objectIndex, m_level, m_threadId;
	bool m_dirtyStart, m_dirtyEnd;
	const DataNode* m_node;
};

#ifdef PANDA_LOG_EVENTS

// To log an event, you only have to use this class
class PANDA_CORE_API ScopedEvent
{
public:
	ScopedEvent(EventType type, const PandaObject* object);
	ScopedEvent(EventType type, const BaseData* data);
	ScopedEvent(const std::string& text, DataNode* node = nullptr);
	~ScopedEvent();

private:
	EventData m_event;
	bool m_changeLevel; // Do we have to change the level back in the destructor
};

#else

// This version does nothing, it should not slow down the code using it
class PANDA_CORE_API ScopedEvent
{
public:
	ScopedEvent(EventType type, const PandaObject* object) {}
	ScopedEvent(EventType type, const BaseData* data) {}
	ScopedEvent(const std::string& text, DataNode* node = nullptr) {}
};

#endif

// Container for all the events
class PANDA_CORE_API UpdateLogger
{
public:
	typedef std::vector<EventData> UpdateEvents;
	typedef std::map<const DataNode*, bool> NodeStates;

	static UpdateLogger* getInstance();
	static int getThreadId();

	void startLog(PandaDocument* doc);
	void stopLog();
	void updateDirtyStates(); // Necessary when a single event can modify the status of many nodes (like the scheduler)

	void setNbThreads(int nbThreads);
	int getNbThreads() const;
	void setupThread(int id);

	const UpdateEvents getEvents(int id) const;
	const NodeStates getInitialNodeStates() const;

protected:
	UpdateLogger();
	friend class ScopedEvent;
	friend class Scheduler;

	void addEvent(EventData event);
	int& logLevel(int threadId);

	std::vector<UpdateEvents> m_events, m_prevEvents;
	int m_nbThreads;
	bool m_logging;
	NodeStates m_nodeStates, m_prevNodeStates;
	std::vector<int> m_logLevelMap;
	PandaDocument* m_document;
};

inline int& UpdateLogger::logLevel(int threadId)
{ return m_logLevelMap[threadId]; }

inline int UpdateLogger::getNbThreads() const
{ return m_nbThreads; }

} // namespace helper

} // namespace panda

#endif // UPDATELOGGER_H
