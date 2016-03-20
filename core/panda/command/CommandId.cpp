#include <panda/command/CommandId.h>

#include <map>
#include <typeindex>

namespace panda
{

using CommandIdMap = std::map<std::type_index, int>;

static CommandIdMap& getCommandIdMap()
{
	static CommandIdMap commandIdMap;
	return commandIdMap;
}

int getCommandId(const std::type_info& type)
{
	CommandIdMap& commandIdMap = getCommandIdMap();
	std::type_index index(type);
	if(commandIdMap.count(index))
		return commandIdMap.at(index);
	else
	{
		static int i = 1; // start at 1
		commandIdMap[index] = i;
		++i;
		return i-1;
	}
}

} // namespace panda
