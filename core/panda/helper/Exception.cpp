#include <panda/helper/Exception.h>

namespace panda
{

namespace helper
{

	
Exception::Exception(const std::string& message)
	: m_message(message)
{
}

const char* Exception::what() const noexcept
{
	return m_message.c_str();
}

} // namespace helper

} // namespace panda
