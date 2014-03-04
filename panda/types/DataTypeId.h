#ifndef DATATYPEID_H
#define DATATYPEID_H

#include <QMap>
#include <typeindex>
#include <typeinfo>

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
	typedef QMap<std::type_index, int> TypesIdMap;
	static TypesIdMap& getTypesIdMap();

	// To register vectors and animations
	static void registerType(const std::type_info& type, int fullType);
	template <class T>
	static void registerType(int fullType) { registerType(typeid(T), fullType); }
};

} // namespace types

} // namespace panda

#endif // DATATYPEID_H
