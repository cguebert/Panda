#include <panda/types/TypeConverter.h>
#include <map>

namespace panda
{

namespace types
{

typedef std::map<int, std::map<int, TypeConverter::FunctorPtr> > FunctorMap;

static FunctorMap& getFunctorMap()
{
	static FunctorMap theMap;
	return theMap;
}

bool TypeConverter::canConvert(int fromType, int toType)
{
	const FunctorMap& theMap = getFunctorMap();
	if(theMap.find(fromType) == theMap.end())
		return false;
	const auto& typeMap = theMap.at(fromType);
	return typeMap.find(toType) != typeMap.end();
}

void TypeConverter::convert(int fromType, int toType, const void* valueFrom, void* valueTo)
{
	const FunctorMap& map1 = getFunctorMap();
	if(map1.find(fromType) == map1.end())
		return;
	const auto& map2 = map1.at(fromType);
	if(map2.find(toType) == map2.end())
		return;

	auto functor = map2.at(toType);
	functor->convert(valueFrom, valueTo);
}

void TypeConverter::registerFunctor(int fromType, int toType, FunctorPtr ptr)
{
	FunctorMap& map = getFunctorMap();
	map[fromType][toType] = ptr;
}

} // namespace types

} // namespace panda
