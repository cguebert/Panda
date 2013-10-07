#ifndef DATA_INL
#define DATA_INL

#include <panda/Data.h>

namespace panda
{

template<class T>
Data<T>::Data(const BaseData::BaseInitData& init)
	: BaseData(init)
	, value(value_type())
{
	initFlags();
}

template<class T>
Data<T>::Data(const InitData& init)
	: BaseData(init)
{
	value = init.value;
	initFlags();
}

template<class T>
Data<T>::Data(const QString& name, const QString& help, PandaObject* owner)
	: BaseData(name, help, owner)
	, value(T())
{
	initFlags();
}

template<class T>
Data<T>::~Data()
{
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
	updateIfDirty();
	return value;
}

template<class T>
void Data<T>::copyValueFrom(const BaseData* from)
{
	if(dataCopier->copyData(this, from))
		isValueSet = true;
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
