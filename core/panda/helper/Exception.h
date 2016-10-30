#pragma once

#include <panda/core.h>

#include <string>

namespace panda
{

namespace helper
{

class PANDA_CORE_API Exception: public std::exception
{
public: 
	Exception(const std::string& message);
		
	const char* what() const noexcept override;

protected:
	std::string m_message;
};

} // namespace helper

} // namespace panda
