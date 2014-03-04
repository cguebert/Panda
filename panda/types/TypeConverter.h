#ifndef TYPECONVERTER_H
#define TYPECONVERTER_H

#include <panda/types/DataTypeId.h>

#include <QSharedPointer>

namespace panda
{

namespace types
{

class BaseConverterFunctor
{
public:
	virtual void convert(const void* valueFrom, void* valueTo) const = 0;
};

class TypeConverter
{
public:
	static bool canConvert(int fromType, int toType);
	static void convert(int fromType, int toType, const void* valueFrom, void* valueTo);

	typedef QSharedPointer<BaseConverterFunctor> FunctorPtr;
private:
	template<class F, class T> friend class RegisterTypeConverter;

	static void registerFunctor(int fromType, int toType, FunctorPtr ptr);
};

template<class From, class To>
class RegisterTypeConverter
{
public:
	typedef void convertType(const From& valueFrom, To& valueTo);

	RegisterTypeConverter()
	{
		int fromType = DataTypeId::getIdOf<From>();
		int toType = DataTypeId::getIdOf<To>();
		QSharedPointer<BaseConverterFunctor> functor(new TypeConverterGlobal<From, To>);

		TypeConverter::registerFunctor(fromType, toType, functor);
	}

	RegisterTypeConverter(convertType* function)
	{
		int fromType = DataTypeId::getIdOf<From>();
		int toType = DataTypeId::getIdOf<To>();
		QSharedPointer<BaseConverterFunctor> functor(new TypeConverterFunctor<From, To>(function));

		TypeConverter::registerFunctor(fromType, toType, functor);
	}
};

template<class From, class To>
class TypeConverterFunctor : public BaseConverterFunctor
{
public:
	typedef void convertType(const From& valueFrom, To& valueTo);

	TypeConverterFunctor(convertType* function) : convertFunction(function) {}
	virtual void convert(const void* valueFrom, void* valueTo) const
	{
		(*convertFunction)(*static_cast<const From*>(valueFrom), *static_cast<To*>(valueTo));
	}

private:
	convertType* convertFunction;
};

template<class From, class To>
class TypeConverterGlobal : public BaseConverterFunctor
{
public:
	virtual void convert(const void* valueFrom, void* valueTo) const
	{
		::convertType(*static_cast<const From*>(valueFrom), *static_cast<To*>(valueTo));
	}
};

} // namespace types

} // namespace panda

#endif // TYPECONVERTER_H
