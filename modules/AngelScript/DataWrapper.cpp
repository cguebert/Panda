#include "DataWrapper.h"

#include <panda/types/Gradient.h>
#include <panda/types/Rect.h>

#include <cassert>
#include <angelscript.h>

namespace panda 
{

inline const char* str(const std::string& text)
{
	return text.c_str();
}

template <class T>
void registerDataType(asIScriptEngine* engine, const std::string& typeName)
{
	const std::string dataTypeName = "Data<" + typeName + ">";
	auto dtn = dataTypeName.c_str();
	int r = 0;
	r = engine->RegisterObjectType(dtn, 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str(typeName + " getValue()"),
		asMETHOD(panda::DataWrapper<T>, getValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, str("void setValue(const " + typeName + " &in)"),
		asMETHOD(panda::DataWrapper<T>, setValue), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod(dtn, "int getCounter()",
		asMETHOD(panda::DataWrapper<T>, getCounter), asCALL_THISCALL); assert(r >= 0);
}

void registerData(asIScriptEngine* engine)
{
	int r = 0;
	r = engine->RegisterObjectType("Data<class T>", 0, asOBJ_REF | asOBJ_NOCOUNT | asOBJ_TEMPLATE); assert(r >= 0);

	registerDataType<int>(engine, "int");
	registerDataType<float>(engine, "float");
	registerDataType<types::Point>(engine, "Point");
	registerDataType<types::Rect>(engine, "Rect");
}

} // namespace panda
