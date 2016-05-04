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

class BaseDataWrapper 
{
public:
	BaseDataWrapper(BaseData* data)
		: m_baseData(data) { }

	int getCounter() const 
	{ return m_baseData->getCounter(); }

	void setWidget(const std::string& widget)
	{ m_baseData->setWidget(widget); }

	void setWidgetData(const std::string& widgetData)
	{ m_baseData->setWidgetData(widgetData); }

private:
	BaseData* m_baseData = nullptr;
};

template <class T, class Ref = const T&>
class DataWrapper : public BaseDataWrapper
{
public:
	DataWrapper(Data<T>* data, asIScriptEngine* /*engine*/) 
		: BaseDataWrapper(data), m_data(data) { }

	Ref getValue() const 
	{ return m_data->getValue(); }
	
	void setValue(Ref value) 
	{ m_data->setValue(value); }
	
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

	VectorDataWrapper(data_type* data, asIScriptEngine* /*engine*/)
		: BaseDataWrapper(data), m_data(data) { }

	script_vector* getValue() const
	{
		auto vec = new script_vector();
		vec->container = m_data->getValue();
		return vec;
	}
	
	void setValue(const script_vector* vec)
	{ m_data->setValue(vec->container); }

private:
	data_type* m_data = nullptr;
};

template <class T>
BaseDataWrapper* castToBase(T* data)
{
	return dynamic_cast<BaseDataWrapper*>(data);
}

template <class T>
void registerBaseDataMethods(asIScriptEngine* engine, const char* dtn)
{
	int r = 0;
	r = engine->RegisterObjectMethod(dtn, "int getCounter()",
		asMETHOD(T, getCounter), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, "void setWidget(const string &in widget)",
		asMETHOD(T, setWidget), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, "void setWidgetData(const string &in widgetData)",
		asMETHOD(T, setWidgetData), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectMethod(dtn, "BaseData@ opImplCast()", asFUNCTION(castToBase<T>), asCALL_CDECL_OBJLAST); assert(r >= 0);
}

template <class T>
void registerDataType(asIScriptEngine* engine, const std::string& typeName)
{
	std::string capitalizedTypeName = typeName;
	capitalizedTypeName[0] = toupper(capitalizedTypeName[0]);
	const std::string dataTypeName = capitalizedTypeName + "Data";
	auto dtn = dataTypeName.c_str();
	int r = 0;
	r = engine->RegisterObjectType(dtn, 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("const " + typeName + "& getValue() const"),
		asMETHOD(panda::DataWrapper<T>, getValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("void setValue(const " + typeName + " &in value)"),
		asMETHOD(panda::DataWrapper<T>, setValue), asCALL_THISCALL); assert(r >= 0);
	registerBaseDataMethods<panda::DataWrapper<T>>(engine, dtn);
}

template <class T>
void registerVectorDataType(asIScriptEngine* engine, const std::string& typeName)
{
	std::string capitalizedTypeName = typeName;
	capitalizedTypeName[0] = toupper(capitalizedTypeName[0]);
	const std::string dataTypeName = capitalizedTypeName + "VectorData";
	auto dtn = dataTypeName.c_str();
	int r = 0;
	r = engine->RegisterObjectType(dtn, 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("vector< " + typeName + ">@ getValue() const"),
		asMETHOD(panda::VectorDataWrapper<T>, getValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("void setValue(const vector<" + typeName + "> &in value)"),
		asMETHOD(panda::VectorDataWrapper<T>, setValue), asCALL_THISCALL); assert(r >= 0);
	registerBaseDataMethods<panda::VectorDataWrapper<T>>(engine, dtn);
}

} // namespace panda
