#include <panda/types/DataTraits.h>
#include <panda/types/DataTypeId.h>

#include <map>
#include <typeindex>

namespace panda
{

namespace types
{

typedef std::map<int, AbstractDataTrait*> TraitsMap;

static TraitsMap& getTraitsMap()
{
	static TraitsMap traitsMap;
	return traitsMap;
}

AbstractDataTrait* DataTraitsList::getTrait(int fullTypeId)
{
	const TraitsMap& traitsMap = getTraitsMap();
	if(traitsMap.count(fullTypeId))
		return traitsMap.at(fullTypeId);
	else
		return nullptr;
}

AbstractDataTrait* DataTraitsList::getTrait(const std::type_info& type)
{
	return getTrait(DataTypeId::getId(type));
}

void DataTraitsList::registerTrait(AbstractDataTrait* trait)
{
	TraitsMap& traitsMap = getTraitsMap();
	int index = trait->fullTypeId();
	if(!traitsMap.count(index))
		traitsMap.emplace(index, trait);
}

} // namespace types

} // namespace panda
