#include <panda/BaseClass.h>

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

QString BaseClass::getTypeName() const
{
	return m_typeName;
}

QString BaseClass::getNamespaceName() const
{
	return m_namespaceName;
}

QString BaseClass::getClassName() const
{
	return m_className;
}

QString BaseClass::getTemplateName() const
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

QString BaseClass::decodeTypeName(const std::type_info& type)
{
	QString name;
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
	name.replace("class ", "");
	name.replace("struct ", "");
	name.replace(" ", "");

#ifdef PANDA_DOUBLE
	name.replace("<double>", "<PReal>");
#else
	name.replace("<float>", "<PReal>");
#endif
	return name;
}

QString BaseClass::decodeClassName(const std::type_info& type)
{
	QString realName = decodeTypeName(type);
	int templateStart = realName.indexOf("<");
	int start = realName.lastIndexOf("::", templateStart);
	if(start < 0)
		start = 0;
	else
		start += 2;
	int end = realName.indexOf("<", start);
	if(end < 0)
		end = realName.length();

	return realName.mid(start, end-start);
}

QString BaseClass::decodeNamespaceName(const std::type_info& type)
{
	QString realName = decodeTypeName(type);
	int templateStart = realName.indexOf("<");
	int end = realName.lastIndexOf("::", templateStart);
	if(end < 0)
		return "";
	else
		return realName.left(end);
}

QString BaseClass::decodeTemplateName(const std::type_info& type)
{
	QString realName = decodeTypeName(type);
	int start = realName.indexOf("<");
	if(start < 0)
		start = 0;
	else
		++start;
	int end = realName.lastIndexOf(">");
	if(end < 0)
		end = realName.length();

	return realName.mid(start, end-start);
}

} // namespace Panda
