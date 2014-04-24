#ifndef TYPES_DATATYPEID_H
#define TYPES_DATATYPEID_H

#include <typeinfo>

template<class T> class RegisterWidget;

namespace panda
{

template<class T> class RegisterData;

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

	template<class T> friend class RegisterData;
	template<class T> friend class RegisterWidget;
	template<class From, class To> friend class RegisterTypeConverter;

	// To register vectors and animations
	static void registerType(const std::type_info& type, int fullType);
	template <class T>
	static void registerType(int fullType) { registerType(typeid(T), fullType); }
};

} // namespace types

} // namespace panda

#endif // TYPES_DATATYPEID_H
