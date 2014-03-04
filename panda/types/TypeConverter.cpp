#include <panda/types/TypeConverter.h>
#include <QMap>

namespace panda
{

namespace types
{

typedef QMap<int, QMap<int, TypeConverter::FunctorPtr> > FunctorMap;

static FunctorMap& getFunctorMap()
{
	static FunctorMap theMap;
	return theMap;
}

bool TypeConverter::canConvert(int fromType, int toType)
{
	const FunctorMap& theMap = getFunctorMap();
	if(!theMap.contains(fromType))
		return false;
	return theMap[fromType].contains(toType);
}

void TypeConverter::convert(int fromType, int toType, const void* valueFrom, void* valueTo)
{
	const FunctorMap& map1 = getFunctorMap();
	if(!map1.contains(fromType))
		return;
	const auto& map2 = map1[fromType];
	if(!map2.contains(toType))
		return;

	auto functor = map2[toType];
	functor->convert(valueFrom, valueTo);
}

void TypeConverter::registerFunctor(int fromType, int toType, FunctorPtr ptr)
{
	FunctorMap& map = getFunctorMap();
	map[fromType][toType] = ptr;
}

} // namespace types

} // namespace panda
