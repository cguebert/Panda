#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <panda/DataAccessor.h>

namespace panda
{

template<class T> class DataAccessor;
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
	typedef DataAccessor<data_type> data_accessor;

	class InitData : public BaseData::BaseInitData
	{
	public:
		InitData() : value(value_type()) {}
		value_type value;
	};

	explicit Data(const BaseData::BaseInitData& init);
	explicit Data(const InitData& init);
	Data(const QString& name, const QString& help, PandaObject* owner);
	virtual ~Data();

	virtual void update();
	virtual void setDirtyValue(const DataNode* caller);

	virtual void setParent(BaseData* parent);
	virtual const types::AbstractDataTrait* getDataTrait() const;
	virtual const void* getVoidValue() const;
	data_accessor getAccessor(); /// Return a wrapper around the pointer to the value (call endEdit in the destructor)
	inline void setValue(const_reference value); /// Store value in this Data
	inline const_reference getValue() const; /// Retrieve the stored value
	virtual void copyValueFrom(const BaseData* from);

	virtual int getCounter() const;

protected:
	friend class DataAccessor<data_type>;
	friend class RegisterData<value_type>;

	inline pointer beginEdit();
	inline void endEdit();
	virtual void* beginVoidEdit();
	virtual void endVoidEdit();

private:
	value_type m_value;
	Data<value_type>* m_parentData;
	static types::AbstractDataTrait* m_dataTrait;
	static AbstractDataCopier* m_dataCopier;

	Data();
	Data(const Data&);
	Data& operator=(const Data&);
};

//****************************************************************************//

template<class T>
class DataAccessor<Data<T>> : public DataAccessor<T>
{
public:
	typedef DataAccessor<T> Inherit;
	typedef panda::Data<T> data_type;
	typedef T container_type;

protected:
	data_type& m_data;

public:
	DataAccessor(data_type& data) : Inherit(*data.beginEdit()) , m_data(data) {}
	~DataAccessor() { m_data.endEdit(); }

	template<class U> void operator=(const U& value) { Inherit::operator=(value); }

	/// DataAccessor is noncopyable
	DataAccessor(const DataAccessor&) = delete;
	DataAccessor& operator=(const DataAccessor&) = delete;
};

//****************************************************************************//

template<class T>
inline void* Data<T>::beginVoidEdit()
{ return beginEdit(); }

template<class T>
inline void Data<T>::endVoidEdit()
{ endEdit(); }

template<class T>
inline const void* Data<T>::getVoidValue() const
{ return &getValue(); }

template<class T>
inline void Data<T>::setValue(const_reference value)
{
	*beginEdit() = value;
	endEdit();
}

template<class T>
inline typename Data<T>::pointer Data<T>::beginEdit()
{
	updateIfDirty();
	++m_counter;
	return &m_value;
}

template<class T>
inline void Data<T>::endEdit()
{
	m_isValueSet = true;
	BaseData::setDirtyOutputs();
}

} // namespace panda

#endif // DATA_H
