#ifndef DATACOPIER_H
#define DATACOPIER_H

#include <panda/Data.h>
#include <panda/types/DataTraits.h>
#include <panda/types/Animation.h>

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

//***************************************************************//

template<class T>
bool DataCopier<T>::copyData(Data<T> *dest, const BaseData* from)
{
	typedef types::DataTrait<T> DestTrait;
	auto fromTrait = from->getDataTrait();
	// First we try without conversion
	if(fromTrait->isVector())
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
	else if(fromTrait->isSingleValue())
	{
		// Same type
		const Data<T>* castedFrom = dynamic_cast<const Data<T>*>(from);
		if(castedFrom)
		{
			dest->setValue(castedFrom->getValue());
			return true;
		}
	}

	// Else we try a conversion
	if(DestTrait::isNumerical() && fromTrait->isNumerical())
	{
		auto value = dest->getAccessor();
		DestTrait::setNumerical(value.wref(), fromTrait->getNumerical(from->getVoidValue(), 0), 0);
		return true;
	}

	return false;
}

//***************************************************************//

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
		if(DestTrait::isNumerical() && fromTrait->isNumerical())
		{
			auto fromValue = from->getVoidValue();
			auto value = dest->getAccessor();
			int size = fromTrait->size(fromValue);
			value.resize(size);
			for(int i=0; i<size; ++i)
				DestTrait::setNumerical(value.wref(), fromTrait->getNumerical(fromValue, i), i);
			return true;
		}

		return false;
	}
};

//***************************************************************//

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
