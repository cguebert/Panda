#include <panda/DataCopier.h>
#include <panda/types/DataTypeId.h>

#include <QMap>
#include <typeindex>

namespace panda
{

typedef QMap<int, AbstractDataCopier*> CopiersMap;

static CopiersMap& getCopiersMap()
{
	static CopiersMap copiersMap;
	return copiersMap;
}

AbstractDataCopier* DataCopiersList::getCopier(int fullTypeId)
{
	const CopiersMap& copiersMap = getCopiersMap();
	if(copiersMap.contains(fullTypeId))
		return copiersMap.value(fullTypeId);
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
	if(!copiersMap.contains(fullTypeId))
		copiersMap[fullTypeId] = copier;
}

} // namespace panda
