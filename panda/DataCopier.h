#ifndef DATACOPIER_H
#define DATACOPIER_H

#include <panda/Data.h>
#include <panda/types/DataTraits.h>
#include <panda/types/Animation.h>
#include <panda/types/TypeConverter.h>

namespace panda
{

template<class T>
class DataCopier
{
public:
	static bool copyData(Data<T>* dest, const BaseData* from);
};

template<class T>
class VirtualDataCopier : public AbstractDataCopier
{
public:
	typedef T value_type;
	static VirtualDataCopier* get() { static VirtualDataCopier<value_type> copier; return &copier; }

	virtual bool copyData(BaseData* dest, const BaseData* from) const
	{
		typedef Data<value_type> data_type;
		return DataCopier<T>::copyData(static_cast<data_type*>(dest), from);
	}
};

//****************************************************************************//

template<class T>
bool DataCopier<T>::copyData(Data<T> *dest, const BaseData* from)
{
	typedef types::DataTrait<T> DestTrait;
	auto fromTrait = from->getDataTrait();

	// First we try without conversion
	if(fromTrait->isSingleValue())
	{
		// Same type
		const Data<T>* castedFrom = dynamic_cast<const Data<T>*>(from);
		if(castedFrom)
		{
			dest->setValue(castedFrom->getValue());
			return true;
		}
	}
	else if(fromTrait->isVector())
	{
		// The from is a vector of T
		const Data< QVector<T> >* castedVectorFrom = dynamic_cast<const Data< QVector<T> >*>(from);
		if(castedVectorFrom)
		{
			if(castedVectorFrom->getValue().size())
				dest->setValue(castedVectorFrom->getValue()[0]);
			else
				dest->setValue(T());
			return true;
		}
	}

	// Else we try a conversion
	// From something X to single value Y
	int fromTypeId = fromTrait->fullTypeId(), destTypeId = DestTrait::fullTypeId();
	if(types::TypeConverter::canConvert(fromTypeId, destTypeId))
	{
		auto toValue = dest->getAccessor();
		types::TypeConverter::convert(fromTypeId, destTypeId, from->getVoidValue(), &toValue.wref());
		return true;
	}

	// From a vector of X to single value Y
	fromTypeId = fromTrait->valueTypeId();
	if(types::TypeConverter::canConvert(fromTypeId, destTypeId))
	{
		const void* fromValuePtr = fromTrait->getVoidValue(from->getVoidValue(), 0);
		if(fromValuePtr)
		{
			auto toValue = dest->getAccessor();
			types::TypeConverter::convert(fromTypeId, destTypeId, fromValuePtr, &toValue.wref());
			return true;
		}
	}

	return false;
}

//****************************************************************************//

template<class T>
class DataCopier< QVector<T> >
{
public:
	typedef T value_type;
	typedef QVector<value_type> vector_type;
	typedef types::Animation<value_type> animation_type;
	typedef Data<vector_type> data_type;

	static bool copyData(data_type* dest, const BaseData* from)
	{
		typedef types::DataTrait<vector_type> DestTrait;
		auto fromTrait = from->getDataTrait();
		// First we try without conversion
		if(fromTrait->isVector())
		{
			// Same type (both vectors)
			const data_type* castedFrom = dynamic_cast<const data_type*>(from);
			if(castedFrom)
			{
				dest->setValue(castedFrom->getValue());
				return true;
			}
		}
		else if(fromTrait->isSingleValue())
		{
			// The from is not a vector of T, but a single value of type T
			const Data<value_type>* castedSingleValueFrom = dynamic_cast<const Data<value_type>*>(from);
			if(castedSingleValueFrom)
			{
				auto vec = dest->getAccessor();
				vec.clear();
				vec.push_back(castedSingleValueFrom->getValue());
				return true;
			}
		}

		// Else we try a conversion
		int fromFullTypeId = fromTrait->fullTypeId(), fromValueTypeId = fromTrait->valueTypeId();
		int destFullTypeId = DestTrait::fullTypeId(), destValueTypeId = DestTrait::valueTypeId();
		// From something X to vector Y
		if(types::TypeConverter::canConvert(fromFullTypeId, destFullTypeId))
		{
			auto toValue = dest->getAccessor();
			types::TypeConverter::convert(fromFullTypeId, destFullTypeId, from->getVoidValue(), &toValue.wref());
			return true;
		}

		// From a single value or vector of X to vector of Y
		if(types::TypeConverter::canConvert(fromValueTypeId, destValueTypeId))
		{
			const void* fromPtr = from->getVoidValue();
			auto toValue = dest->getAccessor();
			int size = fromTrait->size(fromPtr);
			DestTrait::clear(toValue.wref(), size, true);
			for(int i=0; i<size; ++i)
			{
				const void* fromValuePtr = fromTrait->getVoidValue(fromPtr, i);
				void* toValuePtr = DestTrait::getVoidValue(toValue.wref(), i);
				if(fromValuePtr && toValuePtr)
					types::TypeConverter::convert(fromValueTypeId, destValueTypeId, fromValuePtr, toValuePtr);
			}
			return true;
		}

		// From a single value or vector of X to vector Y
		if(types::TypeConverter::canConvert(fromValueTypeId, destFullTypeId))
		{
			const void* fromValuePtr = fromTrait->getVoidValue(from->getVoidValue(), 0);
			if(fromValuePtr)
			{
				auto toValue = dest->getAccessor();
				types::TypeConverter::convert(fromValueTypeId, destFullTypeId, fromValuePtr, &toValue.wref());
				return true;
			}
		}

		// From something X to first value of vector Y
		if(types::TypeConverter::canConvert(fromFullTypeId, destValueTypeId))
		{
			auto toValue = dest->getAccessor();
			DestTrait::clear(toValue.wref(), 1, true);
			void* toValuePtr = DestTrait::getVoidValue(toValue.wref(), 0);
			if(toValuePtr)
			{
				types::TypeConverter::convert(fromFullTypeId, destValueTypeId, from->getVoidValue(), toValuePtr);
				return true;
			}
		}

		return false;
	}
};

//****************************************************************************//

template<class T>
class DataCopier< types::Animation<T> >
{
public:
	typedef T value_type;
	typedef types::Animation<value_type> animation_type;
	typedef Data<animation_type> data_type;

	static bool copyData(data_type* dest, const BaseData* from)
	{
		auto fromTrait = from->getDataTrait();
		// Without conversion
		if(fromTrait->isAnimation())
		{
			// Same type (both animations)
			const data_type* castedAnimationFrom = dynamic_cast<const data_type*>(from);
			if(castedAnimationFrom)
			{
				dest->setValue(castedAnimationFrom->getValue());
				return true;
			}
		}

		// Not accepting conversions from non-animation datas
		dest->getAccessor()->clear();

		return false;
	}
};

} // namespace panda

#endif // DATA_H
