#include <panda/DataTypeId.h>

#include <QMap>

#include <typeindex>

namespace panda
{

int DataTypeId::getId(const std::type_info &type)
{
	static QMap<std::type_index, int> typesId;
	std::type_index index(type);
	if(typesId.contains(index))
		return typesId.value(index);
	else
	{
		int i = typesId.size() + 1; // start at 1
		typesId[index] = i;
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

} // namespace panda
