#include <panda/helper/UpdateLogger.h>

namespace panda
{

namespace helper
{

ScopedEvent::ScopedEvent(EventType type, QString name, quint32 index)
{
	m_event.m_type = type;
	m_event.m_name = name;
	m_event.m_index = index;
	m_event.m_start = std::chrono::high_resolution_clock::now();
	++UpdateLogger::getInstance()->m_level;
}

ScopedEvent::~ScopedEvent()
{
	auto* logger = UpdateLogger::getInstance();
	m_event.m_end = std::chrono::high_resolution_clock::now();
	m_event.m_level = --logger->m_level;
	logger->addEvent(m_event);
}

//***************************************************************//

UpdateLogger::UpdateLogger()
	: m_level(0)
{
}

UpdateLogger* UpdateLogger::getInstance()
{
	static UpdateLogger instance;
	return &instance;
}

void UpdateLogger::clear()
{
	m_events.clear();
}

const UpdateLogger::UpdateEvents UpdateLogger::getEvents() const
{
	return m_events;
}

void UpdateLogger::addEvent(EventData event)
{
	m_events.push_back(event);
}

} // namespace helper

} // namespace panda
