#include <helper/Factory.h>

namespace panda
{

namespace helper
{

static QString& factoryLog()
{
	static QString s;
	return s;
}

/// Log classes registered in the factory
void logFactoryRegister(QString baseclass, QString classname, QString key, bool multi)
{
	factoryLog() += baseclass + (multi?" template class ":" class ")
			+ classname + " registered as " + key + "\n";
}

QString getFactoryLog()
{
	return factoryLog();
}

} // namespace helper

} // namespace panda

