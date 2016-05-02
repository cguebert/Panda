#include "Types.h"
#include "DataWrapper.h"
#include "ObjectWrapper.h"

using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace
{

BaseDataWrapper* createIntData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<int>(name, help); }

BaseDataWrapper* createIntVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<std::vector<int>>(name, help); }

BaseDataWrapper* createFloatData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<float>(name, help); }

BaseDataWrapper* createFloatVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<std::vector<float>>(name, help); }

BaseDataWrapper* createStringData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<std::string>(name, help); }

BaseDataWrapper* createStringVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
{ return wrapper->createData<std::vector<std::string>>(name, help); }

}

namespace panda 
{

	void registerInt(asIScriptEngine* engine)
	{
		registerDataType<int>(engine, "int");
		registerVectorDataType<int>(engine, "int");
		int r = engine->RegisterObjectMethod("PandaObject", "IntData@ createIntData(const string &in, const string &in)",
			asFUNCTION(createIntData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "IntVectorData@ createIntVectorData(const string &in, const string &in)",
			asFUNCTION(createIntVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

	void registerFloat(asIScriptEngine* engine)
	{
		registerDataType<float>(engine, "float");
		registerVectorDataType<int>(engine, "float");
		int r = engine->RegisterObjectMethod("PandaObject", "FloatData@ createFloatData(const string &in, const string &in)",
			asFUNCTION(createFloatData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "FloatVectorData@ createFloatVectorData(const string &in, const string &in)",
			asFUNCTION(createFloatVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

	void registerString(asIScriptEngine* engine)
	{
		registerDataType<std::string>(engine, "string");
		registerVectorDataType<int>(engine, "string");
		int r = engine->RegisterObjectMethod("PandaObject", "StringData@ createStringData(const string &in, const string &in)",
			asFUNCTION(createStringData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "StringVectorData@ createStringVectorData(const string &in, const string &in)",
			asFUNCTION(createStringVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

	void registerAllTypes(asIScriptEngine* engine)
	{
		registerColor(engine);
		registerInt(engine);
		registerIntVector(engine);
		registerFloat(engine);
		registerFloatVector(engine);
		registerGradient(engine);
		registerPoint(engine);
		registerRect(engine);
		registerString(engine);
		registerMesh(engine); // After Point
		registerPath(engine); // After Point
		registerPolygon(engine); // After Path

		registerAnimation(engine);
	}

} // namespace panda
