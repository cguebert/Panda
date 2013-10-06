#ifndef DATATYPEID_H
#define DATATYPEID_H

#include <typeinfo>

namespace panda
{

namespace types
{

class DataTypeId
{
public:
	static int getId(const std::type_info& type);
	template <class T>
	static int getIdOf() { return getId(typeid(T)); }

	static int getFullTypeOfSingleValue(int valueType);
	static int getFullTypeOfVector(int valueType);
	static int getFullTypeOfAnimation(int valueType);

	// To decode the number given by DataTrait::getFullType
	static int getValueType(int fullType);
	static bool isSingleValue(int fullType);
	static bool isVector(int fullType);
	static bool isAnimation(int fullType);
	static int replaceValueType(int fullType, int newType);

private:
	DataTypeId();
};

} // namespace types

} // namespace panda

#endif // DATATYPEID_H
