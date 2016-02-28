#include "ObjectWrapper.h"

#include <cassert>
#include <angelscript.h>

namespace panda 
{

void ObjectWrapper::registerEngine(asIScriptEngine* engine)
{
	int r = 0;
	r = engine->RegisterObjectType("PandaObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

	r = engine->RegisterObjectMethod("PandaObject", "void createIntData(bool, const string &in, const string &in)", 
		asMETHOD(ObjectWrapper, createIntData), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("PandaObject", "void createFloatData(bool, const string &in, const string &in)",
		asMETHOD(ObjectWrapper, createFloatData), asCALL_THISCALL); assert(r >= 0);
}

void ObjectWrapper::createIntData(bool input, const std::string& name, const std::string& help)
{
	DataInfo info;
	info.input = input;
	info.data = std::make_shared<Data<int>>(name, help, m_object);
	m_datas.push_back(info);
}

void ObjectWrapper::createFloatData(bool input, const std::string& name, const std::string& help)
{
	DataInfo info;
	info.input = input;
	info.data = std::make_shared<Data<float>>(name, help, m_object);
	m_datas.push_back(info);
}

} // namespace panda
