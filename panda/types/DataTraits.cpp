#include <panda/types/DataTraits.h>

#include <QMap>
#include <typeindex>

namespace panda
{

namespace types
{

typedef QMap<std::type_index, AbstractDataTrait*> TraitsMap;

static TraitsMap& getTraitsMap()
{
	static TraitsMap traitsMap;
	return traitsMap;
}

AbstractDataTrait* DataTraitsList::getTrait(const std::type_info& type)
{
	const TraitsMap& traitsMap = getTraitsMap();
	std::type_index index(type);
	if(traitsMap.contains(index))
		return traitsMap.value(index);
	else
		return nullptr;
}

void DataTraitsList::registerTrait(const std::type_info& type, AbstractDataTrait* trait)
{
	TraitsMap& traitsMap = getTraitsMap();
	std::type_index index(type);
	if(!traitsMap.contains(index))
		traitsMap[index] = trait;
}

} // namespace types

} // namespace panda
