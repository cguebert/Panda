#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <panda/DataTraits.h>
#include <helper/DataAccessor.h>

namespace panda
{

template<class T> class DataAccessor;
template<class T> class Data;

template<class T>
class DataCopier
{
public:
	static bool copyData(Data<T>* dest, const BaseData* from);
};

template <class T = void*>
class Data : public BaseData
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(Data, T), BaseData)
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef Data<value_type> data_type;

	class InitData : public BaseData::BaseInitData
	{
	public:
		InitData() : value(value_type()) {}
		value_type value;
	};

	explicit Data(const BaseData::BaseInitData& init)
		: BaseData(init)
		, value(value_type())
	{
		displayed = DataTrait<value_type>::isDisplayed();
		persistent = DataTrait<value_type>::isPersistent();
	}

	explicit Data(const InitData& init)
		: BaseData(init)
	{
		value = init.value;
		displayed = DataTrait<value_type>::isDisplayed();
		persistent = DataTrait<value_type>::isPersistent();
	}

	Data(const QString& name, const QString& help, PandaObject* owner)
		: BaseData(name, help, owner)
	{
		displayed = DataTrait<value_type>::isDisplayed();
		persistent = DataTrait<value_type>::isPersistent();
	}

	virtual ~Data() {}

	virtual const AbstractDataTrait* getDataTrait() const
	{ return VirtualDataTrait<value_type>::get(); }

	virtual const void* getVoidValue() const
	{ return &getValue(); }

	helper::DataAccessor<data_type> getAccessor()
	{ return helper::DataAccessor<data_type>(*this); }

	inline void setValue(const_reference v)
	{
		*beginEdit() = v;
		endEdit();
	}

	inline const_reference getValue() const
	{
		updateIfDirty();
		return value;
	}

	virtual void copyValueFrom(const BaseData* from)
	{
//		DataTrait<value_type>::copyValue(this, from);

		DataCopier<value_type>::copyData(this, from);
		isValueSet = true;
	}

	virtual void save(QDomDocument& doc, QDomElement& elem)
	{ DataTrait<value_type>::writeValue(doc, elem, value); }

	virtual void load(QDomElement& elem)
	{
		beginEdit();
		DataTrait<value_type>::readValue(elem, value);
		endEdit();
	}

protected:
	friend class helper::DataAccessor<data_type>;

	inline pointer beginEdit()
	{
		updateIfDirty();
		++counter;
		return &value;
	}

	inline void endEdit()
	{
		isValueSet = true;
		BaseData::setDirtyOutputs();
	}

	virtual void* beginVoidEdit()
	{ return beginEdit(); }

	virtual void endVoidEdit()
	{ endEdit(); }

private:
	T value;

	Data();
	Data(const Data&);
	Data& operator=(const Data&);
};

//***************************************************************//

namespace helper
{

template<class T>
class DataAccessor<Data<T>> : public DataAccessor<T>
{
public:
	typedef DataAccessor<T> Inherit;
	typedef panda::Data<T> data_type;
	typedef T container_type;

protected:
	data_type& data;

public:
	DataAccessor(data_type& d) : Inherit(*d.beginEdit()) , data(d) {}
	~DataAccessor() { data.endEdit(); }

	template<class U> void operator=(const U& v) { Inherit::operator=(v); }
};

} // namespace helper

//***************************************************************//

template<class T>
bool DataCopier<T>::copyData(Data<T> *dest, const BaseData* from)
{
	typedef DataTrait<T> DestTrait;
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

template<class T>
class DataCopier< QVector<T> >
{
public:
	static bool copyData(Data< QVector<T> >* dest, const BaseData* from)
	{
		typedef DataTrait< QVector<T> > DestTrait;
		auto fromTrait = from->getDataTrait();
		// First we try without conversion
		if(fromTrait->isVector())
		{
			// Same type (both vectors)
			const Data< QVector<T> >* castedFrom = dynamic_cast<const Data< QVector<T> >*>(from);
			if(castedFrom)
			{
				dest->setValue(castedFrom->getValue());
				return true;
			}
		}
		else if(fromTrait->isAnimation())
		{
			// The from is not a vector of T, but an animation of type T
			const Data< Animation<T> >* castedAnimationFrom = dynamic_cast<const Data< Animation<T> >*>(from);
			if(castedAnimationFrom)
			{
				auto vec = dest->getAccessor();
				vec = castedAnimationFrom->getValue().getValues().toVector();
				return true;
			}
		}
		else if(fromTrait->isSingleValue())
		{
			// The from is not a vector of T, but a single value of type T
			const Data<T>* castedSingleValueFrom = dynamic_cast<const Data<T>*>(from);
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

template<class T>
class DataCopier< Animation<T> >
{
public:
	static bool copyData(Data< Animation<T> >* dest, const BaseData* from)
	{
		auto fromTrait = from->getDataTrait();
		// Without conversion
		if(fromTrait->isAnimation())
		{
			// Same type (both animations)
			const Data< Animation<T> >* castedAnimationFrom = dynamic_cast<const Data< Animation<T> >*>(from);
			if(castedAnimationFrom)
			{
				dest->setValue(castedAnimationFrom->getValue());
				return true;
			}
		}

		// Not accepting conversions from non-animation datas
		dest->getAccessor().clear();

		return false;
	}
};

} // namespace panda

#endif // DATA_H
