#include <panda/helper/UpdateLogger.h>

#include <windows.h>

namespace panda
{

namespace helper
{

ScopedEvent::ScopedEvent(EventType type, QString name, quint32 index)
{
	m_event.m_type = type;
	auto* logger = UpdateLogger::getInstance();
	m_event.m_level = logger->m_objectsStack.size()-1;

	switch(type)
	{
	case event_update:
	case event_render:
		m_event.m_objectName = name;
		break;
	case event_getValue:
	case event_setDirty:
		m_event.m_dataName = name;
		m_event.m_objectName = logger->m_objectsStack.top();
		break;
	}

	logger->m_objectsStack.push(m_event.m_objectName);
	m_event.m_index = index;
	m_event.m_start = UpdateLogger::getTime();
}

ScopedEvent::~ScopedEvent()
{
	auto* logger = UpdateLogger::getInstance();
	m_event.m_end = UpdateLogger::getTime();
	logger->m_objectsStack.pop();
	logger->addEvent(m_event);
}

//***************************************************************//

UpdateLogger::UpdateLogger()
	: m_logging(false)
{
	m_objectsStack.push("Document");
}

UpdateLogger* UpdateLogger::getInstance()
{
	static UpdateLogger instance;
	return &instance;
}

void UpdateLogger::startLog()
{
	if(m_logging)
		stopLog();
	m_events.clear();
	m_objectsStack.clear();
	m_objectsStack.push("Document");
	m_logging = true;
}

void UpdateLogger::stopLog()
{
	m_logging = false;
	m_prevEvents.swap(m_events);
}

const UpdateLogger::UpdateEvents UpdateLogger::getEvents() const
{
	return m_prevEvents;
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
