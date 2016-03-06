#pragma once

#include <panda/data/Data.h>

#include <angelscript.h>
#include <cassert>
#include <vector>

#pragma warning ( push )
#pragma warning ( disable: 4018 )
#include <modules/AngelScript/addons/aatc/aatc_container_vector.hpp>
#pragma warning ( pop )

class asIScriptEngine;

namespace
{

inline const char* str(const std::string& text)
{ return text.c_str(); }

}

namespace panda
{

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

template <class T>
class VectorDataWrapper : public BaseDataWrapper
{
public:
	using value_type = std::vector<T>;
	using data_type = Data<value_type>;
	using script_vector = aatc::container::tempspec::vector<T>;

	VectorDataWrapper(data_type* data)
		: m_data(data) { }

	script_vector* getValue() const
	{
		auto* vec = new script_vector();
		vec->container = m_data->getValue();
		return vec;
	}
	
	void setValue(script_vector* vec)
	{ m_data->setValue(vec->container); }

	int getCounter() const 
	{ return m_data->getCounter(); }
	
private:
	data_type* m_data = nullptr;
};


template <class T>
void registerDataType(asIScriptEngine* engine, const std::string& typeName)
{
	const std::string dataTypeName = "Data<" + typeName + ">";
	auto dtn = dataTypeName.c_str();
	int r = 0;
	r = engine->RegisterObjectType(dtn, 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("const " + typeName + "& getValue()"),
		asMETHOD(panda::DataWrapper<T>, getValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("void setValue(const " + typeName + " &in)"),
		asMETHOD(panda::DataWrapper<T>, setValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, "int getCounter()",
		asMETHOD(panda::DataWrapper<T>, getCounter), asCALL_THISCALL); assert(r >= 0);
}

template <class T>
void registerVectorDataType(asIScriptEngine* engine, const std::string& typeName)
{
	const std::string dataTypeName = "Data<vector<" + typeName + ">>";
	auto dtn = dataTypeName.c_str();
	int r = 0;
	r = engine->RegisterObjectType(dtn, 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("vector< " + typeName + ">@ getValue()"),
		asMETHOD(panda::VectorDataWrapper<T>, getValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("void setValue(vector<" + typeName + ">@)"),
		asMETHOD(panda::VectorDataWrapper<T>, setValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, "int getCounter()",
		asMETHOD(panda::VectorDataWrapper<T>, getCounter), asCALL_THISCALL); assert(r >= 0);
}

} // namespace panda
