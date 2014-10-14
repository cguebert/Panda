#ifndef TYPES_TYPECONVERTER_H
#define TYPES_TYPECONVERTER_H

#include <panda/core.h>
#include <panda/types/DataTypeId.h>
#include <panda/types/DataTraits.h>

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

class PANDA_CORE_API TypeConverter
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
		ensureTypesAreRegistered();
		int fromType = DataTypeId::getIdOf<From>();
		int toType = DataTypeId::getIdOf<To>();
		QSharedPointer<BaseConverterFunctor> functor(new TypeConverterGlobal<From, To>);

		TypeConverter::registerFunctor(fromType, toType, functor);
	}

	RegisterTypeConverter(convertType* function)
	{
		ensureTypesAreRegistered();
		int fromType = DataTypeId::getIdOf<From>();
		int toType = DataTypeId::getIdOf<To>();
		QSharedPointer<BaseConverterFunctor> functor(new TypeConverterFunctor<From, To>(function));

		TypeConverter::registerFunctor(fromType, toType, functor);
	}

protected:
	/// If we create a converter before the 2 types are fully registered, we do it here
	void ensureTypesAreRegistered()
	{
		DataTypeId::registerType<From>(DataTrait<From>::fullTypeId());
		DataTypeId::registerType<To>(DataTrait<To>::fullTypeId());
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

#endif // TYPES_TYPECONVERTER_H
