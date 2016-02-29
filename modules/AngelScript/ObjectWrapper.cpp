#include "ObjectWrapper.h"

#include <panda/types/Gradient.h>
#include <panda/types/Rect.h>

#include <cassert>
#include <angelscript.h>

namespace panda 
{

void ObjectWrapper::registerObject(asIScriptEngine* engine)
{
	int r = 0;
	r = engine->RegisterObjectType("PandaObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

	r = engine->RegisterObjectMethod("PandaObject", "Data<int>@ createIntData(bool, const string &in, const string &in)", 
		asMETHOD(ObjectWrapper, createIntData), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("PandaObject", "Data<float>@ createFloatData(bool, const string &in, const string &in)",
		asMETHOD(ObjectWrapper, createFloatData), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("PandaObject", "Data<Point>@ createPointData(bool, const string &in, const string &in)",
		asMETHOD(ObjectWrapper, createPointData), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("PandaObject", "Data<Rect>@ createRectData(bool, const string &in, const string &in)",
		asMETHOD(ObjectWrapper, createRectData), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("PandaObject", "Data<Color>@ createColorData(bool, const string &in, const string &in)",
		asMETHOD(ObjectWrapper, createColorData), asCALL_THISCALL); assert(r >= 0);
}

template <class T>
BaseDataWrapper* ObjectWrapper::createData(bool input, const std::string& name, const std::string& help)
{
	auto data = std::make_shared<Data<T>>(name, help, m_object);
	auto dataWrapper = std::make_shared<DataWrapper<T>>(data.get());

	DataInfo info;
	info.input = input;
	info.data = data;
	info.dataWrapper = dataWrapper;
	m_datas.push_back(info);

	return dataWrapper.get();
}

BaseDataWrapper* ObjectWrapper::createIntData(bool input, const std::string& name, const std::string& help)
{ return createData<int>(input, name, help); }

BaseDataWrapper* ObjectWrapper::createFloatData(bool input, const std::string& name, const std::string& help)
{ return createData<float>(input, name, help); }

BaseDataWrapper* ObjectWrapper::createPointData(bool input, const std::string& name, const std::string& help)
{ return createData<types::Point>(input, name, help); }

BaseDataWrapper* ObjectWrapper::createRectData(bool input, const std::string& name, const std::string& help)
{ return createData<types::Rect>(input, name, help); }

BaseDataWrapper* ObjectWrapper::createColorData(bool input, const std::string& name, const std::string& help)
{ return createData<types::Color>(input, name, help); }

} // namespace panda
