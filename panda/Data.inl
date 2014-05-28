#ifndef DATA_INL
#define DATA_INL

#include <panda/Data.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#include <panda/PandaObject.h>
#endif

namespace panda
{

template<class T>
Data<T>::Data(const BaseData::BaseInitData& init)
	: BaseData(init)
	, m_value(value_type())
	, m_parentData(nullptr)
{
	initFlags();
}

template<class T>
Data<T>::Data(const InitData& init)
	: BaseData(init)
	, m_parentData(nullptr)
{
	m_value = init.value;
	initFlags();
}

template<class T>
Data<T>::Data(const QString& name, const QString& help, PandaObject* owner)
	: BaseData(name, help, owner)
	, m_value(T())
	, m_parentData(nullptr)
{
	initFlags();
}

template<class T>
Data<T>::~Data()
{
	// Give connected Datas a chance to copy the value before it is freed
	for(DataNode* node : m_outputs)
		node->doRemoveInput(this);
	m_outputs.clear();
}

template<class T>
void Data<T>::update()
{
	cleanDirty();
	for(DataNode* node : m_inputs)
		node->updateIfDirty();
	if(m_parentData)
	{	// TODO : we shouldn't have to do anything here, there is a bug somewhere...
		beginEdit();
		endEdit();	// As if we touched the data
	}
	else if(m_parentBaseData)
	{
		copyValueFrom(m_parentBaseData);
		cleanDirty(); // Bugfix: copying a value will most often make this Data dirty
	}
}

template<class T>
void Data<T>::setDirtyValue(const DataNode* caller)
{
	if(!isDirty())
	{
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log(helper::event_setDirty, this);
#endif
		DataNode::setDirtyValue(caller);
	}
}

template<class T>
void Data<T>::setParent(BaseData* parent)
{
	Data<T>* TParent = dynamic_cast< Data<T>* >(parent);
	if(m_parentData && !TParent && !m_setParentProtection)	// If deconnecting, we copy the data first
		m_value = m_parentData->getValue();
	m_parentData = TParent;

	BaseData::setParent(parent);
}

template<class T>
const types::AbstractDataTrait* Data<T>::getDataTrait() const
{
	return m_dataTrait;
}

template<class T>
const void* Data<T>::getVoidValue() const
{
	return &getValue();
}

template<class T>
DataAccessor<typename Data<T>::data_type> Data<T>::getAccessor()
{
	return DataAccessor<data_type>(*this);
}

template<class T>
void Data<T>::setValue(const_reference value)
{
	*beginEdit() = value;
	endEdit();
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
void Data<T>::copyValueFrom(const BaseData* from)
{
#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log(helper::event_copyValue, this);
	if(m_dataCopier->copyData(this, from))
		m_isValueSet = true;
#endif
}

template<class T>
typename Data<T>::pointer Data<T>::beginEdit()
{
	updateIfDirty();
	++m_counter;
	return &m_value;
}

template<class T>
void Data<T>::endEdit()
{
	m_isValueSet = true;
	BaseData::setDirtyOutputs();
}

template<class T>
void* Data<T>::beginVoidEdit()
{
	return beginEdit();
}

template<class T>
void Data<T>::endVoidEdit()
{
	endEdit();
}

// Definition of the static members
template<class T> types::AbstractDataTrait* Data<T>::m_dataTrait;
template<class T> AbstractDataCopier* Data<T>::m_dataCopier;

} // namespace panda

#endif // DATA_INL
