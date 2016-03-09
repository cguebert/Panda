#include "Types.h"
#include "DataWrapper.h"
#include "ObjectWrapper.h"

#include <panda/types/Color.h>

#include <angelscript.h>
#include <cassert>
#include <new>

using panda::types::Color;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace
{

// Color constructors
	static void ColorDefaultConstructor(Color* self)
	{ new(self) Color(); }

	static void ColorCopyConstructor(const Color& other, Color* self)
	{ new(self) Color(other); }

	static void Color4FloatsConstructor(float r, float g, float b, float a, Color* self)
	{ new(self) Color(r, g, b, a); }

// Color wrappers
	Color ColorMult(float v, const Color& p)
	{ return p * v; }

	float ColorGetIndex(int idx, Color* self)
	{ if (idx >= 0 || idx < 4) return (*self)[idx]; return 0.f; }

	void ColorSetIndex(int idx, float value, Color* self)
	{ if (idx >= 0 || idx < 4) (*self)[idx] = value; }

	BaseDataWrapper* createColorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Color>(input, name, help); }

	BaseDataWrapper* createColorVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Color>, panda::VectorDataWrapper<Color>>(input, name, help); }

}

namespace panda 
{

	void registerColorType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("Color", sizeof(Color), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS  | asGetTypeTraits<Color>()); assert(r >= 0);
		r = engine->RegisterObjectProperty("Color", "float r", asOFFSET(Color, r)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Color", "float g", asOFFSET(Color, g)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Color", "float b", asOFFSET(Color, b)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Color", "float a", asOFFSET(Color, a)); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(ColorDefaultConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(Color4FloatsConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(const Color &in)", asFUNCTION(ColorCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);

		r = engine->RegisterObjectMethod("Color", "void set(float, float)", asMETHODPR(Color, set, (float, float, float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "void set(const Color &in)", asMETHODPR(Color, set, (const Color&), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "void get(float &out, float &out, float &out, float &out)", asMETHOD(Color, get), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "float get_opIndex(int) const", asFUNCTION(ColorGetIndex), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "void set_opIndex(int, float) const", asFUNCTION(ColorSetIndex), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "void getHsv(float &out, float &out, float &out, float &out)", asMETHOD(Color, getHsv), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterGlobalFunction("Color fromHsv(float, float, float, float)", asFUNCTION(Color::fromHsv), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color premultiplied() const", asMETHOD(Color, premultiplied), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color bounded() const", asMETHOD(Color, bounded), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color opAdd(const Color &in) const", asMETHODPR(Color, operator+, (const Color &) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color &opAddAssign(const Color &in)", asMETHODPR(Color, operator+=, (const Color &), Color&), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color opSub(const Color &in) const", asMETHODPR(Color, operator-, (const Color &) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color &opSubAssign(const Color &in)", asMETHODPR(Color, operator-=, (const Color &), Color&), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color opMul(const Color &in) const", asMETHODPR(Color, operator*, (const Color&) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color opMul(float) const", asMETHODPR(Color, operator*, (float) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color opMul_r(float) const", asFUNCTION(ColorMult), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color &opMulAssign(const Color &in)", asMETHODPR(Color, operator*=, (const Color&), Color&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color &opMulAssign(float)", asMETHODPR(Color, operator*=, (float), Color&), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color opDiv(const Color &in) const", asMETHODPR(Color, operator/, (const Color&) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color opDiv(float) const", asMETHODPR(Color, operator/, (float) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color &opDivAssign(const Color &in)", asMETHODPR(Color, operator/=, (const Color&), Color&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color &opDivAssign(float)", asMETHODPR(Color, operator/=, (float), Color&), asCALL_THISCALL); assert( r >= 0 );
	}

	void registerColor(asIScriptEngine* engine)
	{
		registerColorType(engine);
		registerDataType<Color>(engine, "Color");

		aatc::container::tempspec::vector<Color>::Register(engine, "Color");
		registerVectorDataType<Color>(engine, "Color");
		int r = engine->RegisterObjectMethod("PandaObject", "ColorData@ createColorData(bool, const string &in, const string &in)",
			asFUNCTION(createColorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "ColorVectorData@ createColorVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createColorVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
