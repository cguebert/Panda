#include <panda/types/DataTypeId.h>

namespace panda
{

namespace types
{

int DataTypeId::getId(const std::type_info &type)
{
	TypesIdMap& typesIdMap = getTypesIdMap();
	std::type_index index(type);
	if(typesIdMap.contains(index))
		return typesIdMap.value(index);
	else
	{
		int i = typesIdMap.size() + 1; // start at 1
		typesIdMap[index] = i;
		return i;
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

DataTypeId::TypesIdMap& DataTypeId::getTypesIdMap()
{
	static TypesIdMap typesIdMap;
	return typesIdMap;
}

} // namespace types

} // namespace panda
