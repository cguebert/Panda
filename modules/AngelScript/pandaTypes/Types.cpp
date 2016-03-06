#include "Types.h"
#include "DataWrapper.h"
#include "ObjectWrapper.h"

using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace
{

BaseDataWrapper* createIntData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<int>(input, name, help); }

BaseDataWrapper* createIntVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<std::vector<int>>(input, name, help); }

BaseDataWrapper* createFloatData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<float>(input, name, help); }

BaseDataWrapper* createFloatVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<std::vector<float>>(input, name, help); }

BaseDataWrapper* createStringData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<std::string>(input, name, help); }

BaseDataWrapper* createStringVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<std::vector<std::string>>(input, name, help); }

}

namespace panda 
{

	void registerInt(asIScriptEngine* engine)
	{
		registerDataType<int>(engine, "int");
		registerVectorDataType<int>(engine, "int");
		int r = engine->RegisterObjectMethod("PandaObject", "Data<int>@ createIntData(bool, const string &in, const string &in)",
			asFUNCTION(createIntData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "Data<vector<int>>@ createIntVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createIntVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

	void registerFloat(asIScriptEngine* engine)
	{
		registerDataType<float>(engine, "float");
		registerVectorDataType<int>(engine, "float");
		int r = engine->RegisterObjectMethod("PandaObject", "Data<float>@ createFloatData(bool, const string &in, const string &in)",
			asFUNCTION(createFloatData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "Data<vector<float>>@ createFloatVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createFloatVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

	void registerString(asIScriptEngine* engine)
	{
		registerDataType<std::string>(engine, "string");
		registerVectorDataType<int>(engine, "string");
		int r = engine->RegisterObjectMethod("PandaObject", "Data<string>@ createStringData(bool, const string &in, const string &in)",
			asFUNCTION(createStringData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "Data<vector<string>>@ createStringVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createStringVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

	void registerAllTypes(asIScriptEngine* engine)
	{
		registerColor(engine);
		registerInt(engine);
		registerFloat(engine);
		registerGradient(engine);
		registerPoint(engine);
		registerRect(engine);
		registerString(engine);
	}

} // namespace panda
