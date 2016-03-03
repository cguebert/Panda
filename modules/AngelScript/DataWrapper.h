#pragma once

#include <panda/data/Data.h>
#include "Types.h"

class asIScriptEngine;

namespace panda
{

void registerData(asIScriptEngine* engine);

class GradientWrapper;
class BaseDataWrapper {};

template <class T, class Ref = const T&>
class DataWrapper : public BaseDataWrapper
{
public:
	DataWrapper(Data<T>* data) 
		: m_data(data) { }

	Ref getValue() const 
	{ return m_data->getValue(); }
	
	void setValue(Ref value) 
	{ m_data->setValue(value); }

	int getCounter() const 
	{ return m_data->getCounter(); }
	
private:
	Data<T>* m_data = nullptr;
};

template <>
class DataWrapper<types::Gradient, GradientWrapper*> : public BaseDataWrapper
{
public:
	DataWrapper(Data<types::Gradient>* data) 
		: m_data(data) { }

	GradientWrapper* getValue() const
	{ return GradientWrapper::create(m_data->getValue()); }

	void setValue(GradientWrapper* wrapper)
	{ m_data->setValue(wrapper->gradient()); }

	int getCounter() const 
	{ return m_data->getCounter(); }
	
private:
	Data<types::Gradient>* m_data = nullptr;
};

} // namespace panda
