#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Types.h"

#include <panda/types/Color.h>

#include <new>

using panda::types::Color;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace
{

// Color constructors
	void ColorDefaultConstructor(Color* self)
	{ new(self) Color(); }

	void ColorCopyConstructor(const Color& other, Color* self)
	{ new(self) Color(other); }

	void Color4FloatsConstructor(float r, float g, float b, float a, Color* self)
	{ new(self) Color(r, g, b, a); }

// Color wrappers
	Color ColorMult(float v, const Color& p)
	{ return p * v; }

	float ColorGetIndex(int idx, Color* self)
	{ if (idx >= 0 || idx < 4) return (*self)[idx]; return 0.f; }

	void ColorSetIndex(int idx, float value, Color* self)
	{ if (idx >= 0 || idx < 4) (*self)[idx] = value; }

	BaseDataWrapper* createColorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Color>(name, help); }

	BaseDataWrapper* createColorVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Color>, panda::VectorDataWrapper<Color>>(name, help); }

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
		r = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(float r, float g, float b, float a)", asFUNCTION(Color4FloatsConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("Color", asBEHAVE_CONSTRUCT, "void f(const Color &in color)", asFUNCTION(ColorCopyConstructor), asCALL_CDECL_OBJLAST); assert(r >= 0);

		r = engine->RegisterObjectMethod("Color", "void set(float r, float g, float b, float a)", asMETHODPR(Color, set, (float, float, float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "void set(const Color &in color)", asMETHODPR(Color, set, (const Color&), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "void get(float &out r, float &out g, float &out b, float &out a)", asMETHOD(Color, get), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "float get_opIndex(int index) const", asFUNCTION(ColorGetIndex), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "void set_opIndex(int index, float value) const", asFUNCTION(ColorSetIndex), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "void getHsv(float &out h, float &out s, float &out v, float &out a)", asMETHOD(Color, getHsv), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterGlobalFunction("Color fromHsv(float h, float s, float v, float a)", asFUNCTION(Color::fromHsv), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color premultiplied() const", asMETHOD(Color, premultiplied), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color bounded() const", asMETHOD(Color, bounded), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color opAdd(const Color &in color) const", asMETHODPR(Color, operator+, (const Color &) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color& opAddAssign(const Color &in color)", asMETHODPR(Color, operator+=, (const Color &), Color&), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color opSub(const Color &in color) const", asMETHODPR(Color, operator-, (const Color &) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color& opSubAssign(const Color &in color)", asMETHODPR(Color, operator-=, (const Color &), Color&), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color opMul(const Color &in color) const", asMETHODPR(Color, operator*, (const Color&) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color opMul(float factor) const", asMETHODPR(Color, operator*, (float) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color opMul_r(float factor) const", asFUNCTION(ColorMult), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color& opMulAssign(const Color &in color)", asMETHODPR(Color, operator*=, (const Color&), Color&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color& opMulAssign(float factor)", asMETHODPR(Color, operator*=, (float), Color&), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Color", "Color opDiv(const Color &in color) const", asMETHODPR(Color, operator/, (const Color&) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color opDiv(float factor) const", asMETHODPR(Color, operator/, (float) const, Color), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color& opDivAssign(const Color &in color)", asMETHODPR(Color, operator/=, (const Color&), Color&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Color", "Color& opDivAssign(float factor)", asMETHODPR(Color, operator/=, (float), Color&), asCALL_THISCALL); assert( r >= 0 );
	}

	void registerColor(asIScriptEngine* engine)
	{
		registerColorType(engine);
		registerDataType<Color>(engine, "Color");

		aatc::container::tempspec::vector<Color>::Register(engine, "Color");
		registerVectorDataType<Color>(engine, "Color");
		int r = engine->RegisterObjectMethod("PandaObject", "ColorData@ createColorData(const string &in name, const string &in description)",
			asFUNCTION(createColorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "ColorVectorData@ createColorVectorData(const string &in name, const string &in description)",
			asFUNCTION(createColorVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
