#include <panda/helper/UpdateLogger.h>
#include <panda/PandaObject.h>

#include <windows.h>

namespace panda
{

namespace helper
{

ScopedEvent::ScopedEvent(EventType type, const PandaObject *object)
{
	m_event.m_type = type;
	m_event.m_objectIndex = object->getIndex();
	m_event.m_objectName = object->getName();
	m_event.m_level = ++UpdateLogger::getInstance()->m_level;

	m_event.m_start = UpdateLogger::getTime();
}

ScopedEvent::ScopedEvent(EventType type, const BaseData* data)
{
	m_event.m_type = type;
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

	m_event.m_start = UpdateLogger::getTime();
}

ScopedEvent::~ScopedEvent()
{
	auto* logger = UpdateLogger::getInstance();
	m_event.m_end = UpdateLogger::getTime();
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

void UpdateLogger::startLog()
{
	if(m_logging)
		stopLog();
	m_events.clear();
	m_logging = true;
	m_level = -1;
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
