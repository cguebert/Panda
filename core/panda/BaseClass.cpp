#include <panda/BaseClass.h>
#include <panda/helper/algorithm.h>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

namespace panda
{

BaseClass::BaseClass()
{
}

BaseClass::~BaseClass()
{
}

const std::string& BaseClass::getTypeName() const
{
	return m_typeName;
}

const std::string& BaseClass::getNamespaceName() const
{
	return m_namespaceName;
}

const std::string& BaseClass::getClassName() const
{
	return m_className;
}

const std::string& BaseClass::getTemplateName() const
{
	return m_templateName;
}

bool BaseClass::hasParent(const BaseClass* c) const
{
	if(*this == *c)
		return true;
	unsigned int nb = m_parents.size();
	for(unsigned int i=0; i<nb; ++i)
	{
		if(m_parents[i]->hasParent(c))
			return true;
	}

	return false;
}

bool BaseClass::operator==(const BaseClass& c) const
{
	if(this == &c)
		return true;

	return (getNamespaceName() == c.getNamespaceName())
			&& (getClassName() == c.getClassName())
			&& (getTemplateName() == c.getTemplateName());
}

bool BaseClass::operator!=(const BaseClass& c) const
{
	return !(*this == c);
}

std::string BaseClass::decodeTypeName(const std::type_info& type)
{
	std::string name;
#ifdef __GNUC__
	int status;
	char* allocname = abi::__cxa_demangle(type.name(), 0, 0, &status);
	if(allocname)
		name = allocname;
	else
		std::cerr << "Unable to demangle symbol: " << type.name() << std::endl;
#else
	name = type.name();
#endif
	helper::replaceAll<std::string>(name, "class ", "");
	helper::replaceAll<std::string>(name, "struct ", "");
	helper::replaceAll<std::string>(name, " ", "");

	return name;
}

std::string BaseClass::decodeClassName(const std::type_info& type)
{
	std::string realName = decodeTypeName(type);
	int templateStart = realName.find("<");
	int start = realName.rfind("::", templateStart);
	if(start < 0)
		start = 0;
	else
		start += 2;
	int end = realName.find("<", start);
	if(end < 0)
		end = realName.length();

	return realName.substr(start, end-start);
}

std::string BaseClass::decodeNamespaceName(const std::type_info& type)
{
	std::string realName = decodeTypeName(type);
	int templateStart = realName.find("<");
	int end = realName.rfind("::", templateStart);
	if(end < 0)
		return "";
	else
		return realName.substr(0, end);
}

std::string BaseClass::decodeTemplateName(const std::type_info& type)
{
	std::string realName = decodeTypeName(type);
	int start = realName.find("<");
	if(start < 0)
		start = 0;
	else
		++start;
	int end = realName.rfind(">");
	if(end < 0)
		end = realName.size();

	return realName.substr(start, end-start);
}

} // namespace Panda
