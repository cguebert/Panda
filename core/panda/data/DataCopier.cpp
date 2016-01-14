#include <panda/data/DataCopier.h>
#include <panda/types/DataTypeId.h>

#include <map>
#include <typeindex>

namespace panda
{

typedef std::map<int, AbstractDataCopier*> CopiersMap;

static CopiersMap& getCopiersMap()
{
	static CopiersMap copiersMap;
	return copiersMap;
}

AbstractDataCopier* DataCopiersList::getCopier(int fullTypeId)
{
	const CopiersMap& copiersMap = getCopiersMap();
	if(copiersMap.count(fullTypeId))
		return copiersMap.at(fullTypeId);
	else
		return nullptr;
}

AbstractDataCopier* DataCopiersList::getCopier(const std::type_info& type)
{
	return getCopier(types::DataTypeId::getId(type));
}

void DataCopiersList::registerCopier(int fullTypeId, AbstractDataCopier* copier)
{
	CopiersMap& copiersMap = getCopiersMap();
	if(!copiersMap.count(fullTypeId))
		copiersMap.emplace(fullTypeId, copier);
}

} // namespace panda
