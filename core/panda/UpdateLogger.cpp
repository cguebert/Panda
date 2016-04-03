#include <panda/UpdateLogger.h>
#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>

#include <chrono>
#include <thread>

namespace
{

// Returns a time in nanoseconds (1e-9 seconds)
inline long long getTime()
{ return std::chrono::high_resolution_clock::now().time_since_epoch().count(); }

}


namespace panda
{

namespace helper
{

#ifdef PANDA_LOG_EVENTS

ScopedEvent::ScopedEvent(EventType type, const PandaObject *object)
	: m_changeLevel(true)
{
	m_event.m_type = type;
	m_event.m_node = object;
	m_event.m_objectIndex = object->getIndex();
	m_event.m_text = object->getName();
	m_event.m_threadId = UpdateLogger::getThreadId();
	m_event.m_level = ++UpdateLogger::getInstance()->logLevel(m_event.m_threadId);

	m_event.m_startTime = getTime();
	m_event.m_dirtyStart = object->isDirty();
}

ScopedEvent::ScopedEvent(EventType type, const BaseData* data)
	: m_changeLevel(false)
{
	m_event.m_type = type;
	m_event.m_node = data;
	PandaObject* owner = data->getOwner();
	if(owner)
	{
		m_event.m_objectIndex = owner->getIndex();
		m_event.m_text = owner->getName() + "/" + data->getName();
	}
	else
	{
		m_event.m_objectIndex = -1;
		m_event.m_text = data->getName();
	}
	m_event.m_threadId = UpdateLogger::getThreadId();
	m_event.m_level = UpdateLogger::getInstance()->logLevel(m_event.m_threadId);
	if (m_event.m_level < 0)
		m_event.m_level = 0;

	m_event.m_startTime = getTime();
	m_event.m_dirtyStart = data->isDirty();
}

ScopedEvent::ScopedEvent(const std::string& text, DataNode* node)
	: m_changeLevel(true)
{
	m_event.m_type = event_custom;
	m_event.m_node = node;
	m_event.m_text = text;
	m_event.m_threadId = UpdateLogger::getThreadId();
	m_event.m_level = ++UpdateLogger::getInstance()->logLevel(m_event.m_threadId);

	if(node)
	{
		PandaObject* object = dynamic_cast<PandaObject*>(node);
		if(object)
			m_event.m_objectIndex = object->getIndex();
		else
		{
			BaseData* data = dynamic_cast<BaseData*>(node);
			if(data)
			{
				PandaObject* owner = data->getOwner();
				if(owner)
					m_event.m_objectIndex = owner->getIndex();
			}
		}
	}
	else
		m_event.m_objectIndex = -1;

	m_event.m_startTime = getTime();
	m_event.m_dirtyStart = true;
}

ScopedEvent::~ScopedEvent()
{
	auto logger = UpdateLogger::getInstance();
	m_event.m_endTime = getTime();
	if(m_event.m_node)
		m_event.m_dirtyEnd = m_event.m_node->isDirty();
	else
		m_event.m_dirtyEnd = m_event.m_dirtyStart;

	if(m_changeLevel)
		--logger->logLevel(m_event.m_threadId);

	logger->addEvent(std::move(m_event));
}

#endif // PANDA_LOG_EVENTS

//****************************************************************************//

UpdateLogger::UpdateLogger()
	: m_nbThreads(1)
	, m_logging(false)
	, m_document(nullptr)
{
	m_logLevelMap.push_back(-1);
	m_events.resize(m_nbThreads);
	m_prevEvents.resize(m_nbThreads);
}

UpdateLogger* UpdateLogger::getInstance()
{
	static UpdateLogger instance;
	return &instance;
}

void UpdateLogger::startLog(PandaDocument* doc)
{
	m_document = doc;
	if(m_logging)
		stopLog();

	m_events.resize(m_nbThreads);
	for (auto& events : m_events)
		events.clear();
	
	m_logging = true;

	m_logLevelMap.resize(m_nbThreads, -1);

	m_nodeStates.clear();
	for (auto& object : doc->getObjects())
	{
		m_nodeStates[object.get()] = object->isDirty();

		for(BaseData* data : object->getDatas())
			m_nodeStates[data] = data->isDirty();
	}
}

void UpdateLogger::stopLog()
{
	m_logging = false;
	m_prevEvents.swap(m_events);
	m_prevNodeStates.swap(m_nodeStates);
}

void UpdateLogger::updateDirtyStates()
{
	for (auto& object : m_document->getObjects())
	{
		m_nodeStates[object.get()] = object->isDirty();

		for(BaseData* data : object->getDatas())
			m_nodeStates[data] = data->isDirty();
	}
}

const UpdateLogger::UpdateEvents UpdateLogger::getEvents(int id) const
{
	return m_prevEvents[id];
}

const UpdateLogger::NodeStates UpdateLogger::getInitialNodeStates() const
{
	return m_prevNodeStates;
}

void UpdateLogger::setNbThreads(int nbThreads)
{
	if(m_nbThreads != nbThreads)
	{
		m_events.clear();
		m_events.resize(nbThreads);
		m_nbThreads = nbThreads;
	}
}

typedef std::map<std::thread::id, int> ThreadIdMap;
inline ThreadIdMap& getThreadIdMap()
{
	static ThreadIdMap theMap;
	return theMap;
}

void UpdateLogger::setupThread(int id)
{
	getThreadIdMap()[std::this_thread::get_id()] = id;
}

int UpdateLogger::getThreadId()
{
	return getThreadIdMap()[std::this_thread::get_id()];
}

void UpdateLogger::addEvent(EventData event)
{
	if(m_logging)
		m_events[getThreadId()].push_back(std::move(event));
}

} // namespace helper

} // namespace panda
