#include <panda/command/CommandId.h>

#include <QMap>
#include <typeindex>

typedef QMap<std::type_index, int> CommandIdMap;

static CommandIdMap& getCommandIdMap()
{
	static CommandIdMap commandIdMap;
	return commandIdMap;
}

int getCommandId(const std::type_info& type)
{
	CommandIdMap& commandIdMap = getCommandIdMap();
	std::type_index index(type);
	if(commandIdMap.contains(index))
		return commandIdMap.value(index);
	else
	{
		static int i = 1; // start at 1
		commandIdMap[index] = i;
		++i;
		return i-1;
	}
}
