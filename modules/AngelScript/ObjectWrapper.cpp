#include "ObjectWrapper.h"

#include <cassert>
#include <angelscript.h>

namespace panda 
{

void ObjectWrapper::registerEngine(asIScriptEngine* engine)
{
	int r = 0;
	r = engine->RegisterObjectType("PandaObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

	r = engine->RegisterObjectType("Data<class T>", 0, asOBJ_REF | asOBJ_NOCOUNT | asOBJ_TEMPLATE); assert(r >= 0);

	r = engine->RegisterObjectType("Data<int>", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
	r = engine->RegisterObjectMethod("Data<int>", "int getValue()",
		asMETHOD(DataWrapper<int>, getValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Data<int>", "void setValue(const int &in)",
		asMETHOD(DataWrapper<int>, setValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Data<int>", "int getCounter()",
		asMETHOD(DataWrapper<int>, getCounter), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectType("Data<float>", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
	r = engine->RegisterObjectMethod("Data<float>", "float getValue()",
		asMETHOD(DataWrapper<float>, getValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Data<float>", "void setValue(const float &in)",
		asMETHOD(DataWrapper<float>, setValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Data<float>", "int getCounter()",
		asMETHOD(DataWrapper<float>, getCounter), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectMethod("PandaObject", "Data<int>@ createIntData(bool, const string &in, const string &in)", 
		asMETHOD(ObjectWrapper, createIntData), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("PandaObject", "Data<float>@ createFloatData(bool, const string &in, const string &in)",
		asMETHOD(ObjectWrapper, createFloatData), asCALL_THISCALL); assert(r >= 0);
}

BaseDataWrapper* ObjectWrapper::createIntData(bool input, const std::string& name, const std::string& help)
{
	auto data = std::make_shared<Data<int>>(name, help, m_object);
	auto dataWrapper = std::make_shared<DataWrapper<int>>(data.get());

	DataInfo info;
	info.input = input;
	info.data = data;
	info.dataWrapper = dataWrapper;
	m_datas.push_back(info);

	return dataWrapper.get();
}

BaseDataWrapper* ObjectWrapper::createFloatData(bool input, const std::string& name, const std::string& help)
{
	auto data = std::make_shared<Data<float>>(name, help, m_object);
	auto dataWrapper = std::make_shared<DataWrapper<float>>(data.get());

	DataInfo info;
	info.input = input;
	info.data = data;
	info.dataWrapper = dataWrapper;
	m_datas.push_back(info);

	return dataWrapper.get();
}

} // namespace panda
