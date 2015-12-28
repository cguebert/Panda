#include <panda/types/DataTraits.h>
#include <panda/types/DataTypeId.h>
#include <panda/helper/algorithm.h>

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
	return helper::valueOrDefault(getTraitsMap(), fullTypeId, nullptr);
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
