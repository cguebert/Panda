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
	, value(value_type())
	, parentData(nullptr)
{
	initFlags();
}

template<class T>
Data<T>::Data(const InitData& init)
	: BaseData(init)
	, parentData(nullptr)
{
	value = init.value;
	initFlags();
}

template<class T>
Data<T>::Data(const QString& name, const QString& help, PandaObject* owner)
	: BaseData(name, help, owner)
	, value(T())
	, parentData(nullptr)
{
	initFlags();
}

template<class T>
Data<T>::~Data()
{
	// Give connected Datas a chance to copy the value before it is freed
	for(DataNode* node : outputs)
		node->doRemoveInput(this);
	outputs.clear();
}

template<class T>
void Data<T>::update()
{
	cleanDirty();
	for(DataNode* node : inputs)
		node->updateIfDirty();
	if(parentData)
	{	// TODO : we shouldn't have to do anything here, there is a bug somewhere...
		beginEdit();
		endEdit();	// As if we touched the data
	}
	else if(parentBaseData)
		copyValueFrom(parentBaseData);
}

template<class T>
void Data<T>::setDirtyValue()
{
	if(!dirtyValue)
	{
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log(helper::event_setDirty, this);
#endif
		DataNode::setDirtyValue();
	}
}

template<class T>
void Data<T>::setParent(BaseData* parent)
{
	Data<T>* tmp = dynamic_cast< Data<T>* >(parent);
	if(parentData && !tmp && !setParentProtection)	// If deconnecting, we copy the data first
		value = parentData->getValue();
	parentData = tmp;

	BaseData::setParent(parent);
}

template<class T>
const types::AbstractDataTrait* Data<T>::getDataTrait() const
{
	return dataTrait;
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
void Data<T>::setValue(const_reference v)
{
	*beginEdit() = v;
	endEdit();
}

template<class T>
typename Data<T>::const_reference Data<T>::getValue() const
{
#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log(helper::event_getValue, this);
#endif
	updateIfDirty();
	if(parentData)
		return parentData->getValue();
	return value;
}

template<class T>
void Data<T>::copyValueFrom(const BaseData* from)
{
#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log(helper::event_copyValue, this);
	if(dataCopier->copyData(this, from))
		isValueSet = true;
#endif
}

template<class T>
typename Data<T>::pointer Data<T>::beginEdit()
{
	updateIfDirty();
	++counter;
	return &value;
}

template<class T>
void Data<T>::endEdit()
{
	isValueSet = true;
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
template<class T> types::AbstractDataTrait* Data<T>::dataTrait;
template<class T> AbstractDataCopier* Data<T>::dataCopier;

} // namespace panda

#endif // DATA_INL
