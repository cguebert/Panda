#include <panda/types/DataTypeId.h>

#include <QMap>
#include <typeindex>

namespace panda
{

namespace types
{

typedef QMap<std::type_index, int> TypesIdMap;

static TypesIdMap& getTypesIdMap()
{
	static TypesIdMap typesIdMap;
	return typesIdMap;
}

int DataTypeId::getId(const std::type_info& type)
{
	TypesIdMap& typesIdMap = getTypesIdMap();
	std::type_index index(type);
	if(typesIdMap.contains(index))
		return typesIdMap.value(index);
	else
	{
		static int i = 1; // start at 1
		typesIdMap[index] = i;
		++i;
		return i-1;
	}
}

int DataTypeId::getFullTypeOfSingleValue(int valueType)
{
	return valueType;
}

int DataTypeId::getFullTypeOfVector(int valueType)
{
	return valueType + (1 << 16);
}

int DataTypeId::getFullTypeOfAnimation(int valueType)
{
	return valueType + (1 << 17);
}

int DataTypeId::getValueType(int fullType)
{
	return fullType & 0xFFFF;
}

bool DataTypeId::isSingleValue(int fullType)
{
	return !(fullType & 0xFFFF0000);
}

bool DataTypeId::isVector(int fullType)
{
	return (fullType & (1 << 16)) != 0;
}

bool DataTypeId::isAnimation(int fullType)
{
	return (fullType & (1 << 17)) != 0;
}

int DataTypeId::replaceValueType(int fullType, int newType)
{
	return (fullType & 0xFFFF0000) + newType;
}

void DataTypeId::registerType(const std::type_info& type, int fullType)
{
	TypesIdMap& typesIdMap = getTypesIdMap();
	std::type_index index(type);
	if(!typesIdMap.contains(index))
		typesIdMap[index] = fullType;
}

} // namespace types

} // namespace panda
