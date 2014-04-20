#include <panda/helper/UpdateLogger.h>
#include <panda/PandaObject.h>
#include <panda/PandaDocument.h>

#include <windows.h>

namespace panda
{

namespace helper
{

ScopedEvent::ScopedEvent(EventType type, const PandaObject *object)
{
	m_event.m_type = type;
	m_event.m_node = object;
	m_event.m_objectIndex = object->getIndex();
	m_event.m_objectName = object->getName();
	m_event.m_level = ++UpdateLogger::getInstance()->m_level;

	m_event.m_startTime = UpdateLogger::getTime();
	m_event.m_dirtyStart = object->isDirty();
}

ScopedEvent::ScopedEvent(EventType type, const BaseData* data)
{
	m_event.m_type = type;
	m_event.m_node = data;
	m_event.m_dataName = data->getName();
	PandaObject* object = data->getOwner();
	if(object)
	{
		m_event.m_objectIndex = object->getIndex();
		m_event.m_objectName = object->getName();
	}
	else
		m_event.m_objectIndex = -1;
	m_event.m_level = UpdateLogger::getInstance()->m_level;

	m_event.m_startTime = UpdateLogger::getTime();
	m_event.m_dirtyStart = data->isDirty();
}

ScopedEvent::ScopedEvent(EventType type, int index, QString name)
{
	m_event.m_type = type;
	m_event.m_node = nullptr;
	m_event.m_objectIndex = index;
	m_event.m_objectName = name;
	m_event.m_level = ++UpdateLogger::getInstance()->m_level;

	m_event.m_startTime = UpdateLogger::getTime();
	m_event.m_dirtyStart = true;
}

ScopedEvent::~ScopedEvent()
{
	auto* logger = UpdateLogger::getInstance();
	m_event.m_endTime = UpdateLogger::getTime();
	if(m_event.m_node)
		m_event.m_dirtyEnd = m_event.m_node->isDirty();
	else
		m_event.m_dirtyEnd = false;

	if(m_event.m_dataName.isEmpty())
		--logger->m_level;

	logger->addEvent(m_event);
}

//***************************************************************//

UpdateLogger::UpdateLogger()
	: m_level(-1)
	, m_logging(false)
{
}

UpdateLogger* UpdateLogger::getInstance()
{
	static UpdateLogger instance;
	return &instance;
}

void UpdateLogger::startLog(PandaDocument *doc)
{
	if(m_logging)
		stopLog();
	m_events.clear();
	m_logging = true;
	m_level = -1;

	m_nodeStates.clear();
	for(PandaObject* object : doc->getObjects())
	{
		m_nodeStates[object] = object->isDirty();

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

const UpdateLogger::UpdateEvents UpdateLogger::getEvents() const
{
	return m_prevEvents;
}

const UpdateLogger::NodeStates UpdateLogger::getInitialNodeStates() const
{
	return m_prevNodeStates;
}

void UpdateLogger::addEvent(EventData event)
{
	if(m_logging)
		m_events.push_back(event);
}

unsigned long long UpdateLogger::getTicksPerSec()
{
	LARGE_INTEGER b;
	QueryPerformanceFrequency(&b);
	return(b.QuadPart);
}

unsigned long long UpdateLogger::getTime()
{
	LARGE_INTEGER a;
	QueryPerformanceCounter(&a);
	return(a.QuadPart);
}

} // namespace helper

} // namespace panda
