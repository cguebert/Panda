#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <panda/DataTraits.h>
#include <helper/DataAccessor.h>

namespace panda
{

template<class T> class DataAccessor;

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

	virtual void copyValueFrom(const BaseData* parent)
	{
		DataTrait<value_type>::copyValue(this, parent);
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

} // namespace panda

#endif // DATA_H
