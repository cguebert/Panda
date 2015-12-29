#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <panda/DataAccessor.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

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
	typedef Data<value_type> data_type;
	typedef DataAccessor<data_type> data_accessor;

	explicit Data(const BaseData::BaseInitData& init);
	template<class U> explicit Data(const BaseData::InitData<U>& init);
	Data(const std::string& name, const std::string& help, PandaObject* owner);
	virtual ~Data();

	virtual void update() override;

	virtual void setParent(BaseData* parent) override;
	virtual const void* getVoidValue() const override;
	data_accessor getAccessor(); /// Return a wrapper around the pointer to the value (call endEdit in the destructor)
	inline void setValue(const_reference value); /// Store value in this Data
	inline const_reference getValue() const; /// Retrieve the stored value

	virtual int getCounter() const override;

protected:
	friend class DataAccessor<data_type>;

	inline reference beginEdit();
	inline void endEdit();
	virtual void* beginVoidEdit();
	virtual void endVoidEdit();

private:
	value_type m_value;
	Data<value_type>* m_parentData;

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
	DataAccessor(data_type& data) : Inherit(data.beginEdit()) , m_data(data) {}
	~DataAccessor() { m_data.endEdit(); }

	template<class U> void operator=(const U& value) { Inherit::operator=(value); }

	/// DataAccessor is noncopyable
	DataAccessor(const DataAccessor&) = delete;
	DataAccessor& operator=(const DataAccessor&) = delete;
};

//****************************************************************************//

template<class T>
Data<T>::Data(const BaseData::BaseInitData& init)
	: BaseData(init, typeid(T))
	, m_value(value_type())
	, m_parentData(nullptr)
{
}

template<class T>
template<class U>
Data<T>::Data(const BaseData::InitData<U>& init)
	: BaseData(init, typeid(T))
	, m_parentData(nullptr)
{
	m_value = init.value;
}

template<class T>
Data<T>::Data(const std::string& name, const std::string& help, PandaObject* owner)
	: BaseData(name, help, owner, typeid(T))
	, m_value(T())
	, m_parentData(nullptr)
{
}

template<class T>
Data<T>::~Data()
{
	// Give connected Datas a chance to copy the value before it is freed
	for(DataNode* node : m_outputs)
		node->doRemoveInput(*this);
	m_outputs.clear();
}

template<class T>
void Data<T>::update()
{
	for(DataNode* node : m_inputs)
		node->updateIfDirty();

	if(!m_parentData && m_parentBaseData)
	{
		cleanDirty();
		copyValueFrom(m_parentBaseData);
	}

	cleanDirty();
}

template<class T>
void Data<T>::setParent(BaseData* parent)
{
	// Treating disconnection of a data
	if(!parent && !getFlag(FLAG_SETPARENTPROTECTION))
	{
		if(!isPersistent()) // If the data is not persistent, we reset the value
		{
			m_value = T();
			BaseData::setDirtyValue(this);
		}
		else if(m_parentData)	// Else we copy the data if we never copied it
			m_value = m_parentData->getValue();
	}

	// getValue is optimized when the parent is of the same type as this data
	m_parentData = dynamic_cast< Data<T>* >(parent);

	BaseData::setParent(parent);
}

template<class T>
DataAccessor<typename Data<T>::data_type> Data<T>::getAccessor()
{
	return DataAccessor<data_type>(*this);
}

template<class T>
typename Data<T>::const_reference Data<T>::getValue() const
{
#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log(helper::event_getValue, this);
#endif
	updateIfDirty();
	if(m_parentData)
		return m_parentData->getValue();
	return m_value;
}

template<class T>
int Data<T>::getCounter() const
{
	if(m_parentData)
		return m_parentData->getCounter();
	return BaseData::getCounter();
}

template<class T>
inline void* Data<T>::beginVoidEdit()
{ return &beginEdit(); }

template<class T>
inline void Data<T>::endVoidEdit()
{ endEdit(); }

template<class T>
inline const void* Data<T>::getVoidValue() const
{ return &getValue(); }

template<class T>
inline void Data<T>::setValue(const_reference value)
{
	beginEdit() = value;
	endEdit();
}

template<class T>
inline typename Data<T>::reference Data<T>::beginEdit()
{
	updateIfDirty();
	++m_counter;
	return m_value;
}

template<class T>
inline void Data<T>::endEdit()
{
	forceSet();
	cleanDirty();
	BaseData::setDirtyOutputs();
}

} // namespace panda

#endif // DATA_H
