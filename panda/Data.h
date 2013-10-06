#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <panda/helper/DataAccessor.h>

namespace panda
{

namespace helper { template<class T> class DataAccessor; }
template<class T> class Data;
template<class T> class RegisterData;

class AbstractDataCopier
{
public:
	virtual bool copyData(BaseData* dest, const BaseData* from) const = 0;
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
		initFlags();
	}

	explicit Data(const InitData& init)
		: BaseData(init)
	{
		value = init.value;
		initFlags();
	}

	Data(const QString& name, const QString& help, PandaObject* owner)
		: BaseData(name, help, owner)
	{
		initFlags();
	}

	virtual ~Data() {}

	virtual const types::AbstractDataTrait* getDataTrait() const
	{ return dataTrait; }

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
		if(dataCopier->copyData(this, from))
			isValueSet = true;
	}

protected:
	friend class helper::DataAccessor<data_type>;
	friend class RegisterData<value_type>;

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
	static types::AbstractDataTrait* dataTrait;
	static AbstractDataCopier* dataCopier;

	Data();
	Data(const Data&);
	Data& operator=(const Data&);
};

// Definition of the static members
template<class T> types::AbstractDataTrait* Data<T>::dataTrait;
template<class T> AbstractDataCopier* Data<T>::dataCopier;

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

} // namespace panda

#endif // DATA_H
