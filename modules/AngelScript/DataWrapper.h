#pragma once

#include <panda/data/Data.h>

namespace panda
{

class BaseDataWrapper
{
};

template <class T>
class DataWrapper : public BaseDataWrapper
{
public:
	DataWrapper(Data<T>* data) 
		: m_data(data) { }

	T getValue() const 
	{ return m_data->getValue(); }
	
	void setValue(const T& value) 
	{ m_data->setValue(value); }

	int getCounter() const 
	{ return m_data->getCounter(); }
	
private:
	Data<T>* m_data = nullptr;
};

} // namespace panda
