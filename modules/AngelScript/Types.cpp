#include "Types.h"

#include <panda/types/Gradient.h>
#include <panda/types/Rect.h>

#include <angelscript.h>
#include <cassert>
#include <new>

namespace
{

// Point constructors
	static void PointDefaultConstructor(panda::types::Point* self)
	{ new(self) panda::types::Point(); }

	static void PointCopyConstructor(const panda::types::Point& other, panda::types::Point* self)
	{ new(self) panda::types::Point(other); }

	static void PointInitConstructor(float x, float y, panda::types::Point* self)
	{ new(self) panda::types::Point(x, y); }

	static void PointListConstructor(float* d, panda::types::Point* self)
	{ new(self) panda::types::Point(d); }

// Point wrappers
	panda::types::Point pointMult(float v, const panda::types::Point& p)
	{ return p * v; }
}

namespace panda 
{

	using types::Color;
	using types::Gradient;
	using types::Point;
	using types::Rect;

	void registerPoint(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("Point", sizeof(Point), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS  | asGetTypeTraits<Point>()); assert(r >= 0);
		r = engine->RegisterObjectProperty("Point", "float x", asOFFSET(Point, x)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Point", "float y", asOFFSET(Point, y)); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Point", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(PointDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Point", asBEHAVE_CONSTRUCT, "void f(const Point &in)", asFUNCTION(PointCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Point", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(PointInitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Point", asBEHAVE_LIST_CONSTRUCT, "void f(const int &in) {float, float}", asFUNCTION(PointListConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Point", "Point opAdd(const Point &in) const", asMETHODPR(Point, operator+, (const Point &) const, Point), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point &opAddAssign(const Point &in)", asMETHODPR(Point, operator+=, (const Point &), Point&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point opSub(const Point &in) const", asMETHODPR(Point, operator-, (const Point &) const, Point), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point &opSubAssign(const Point &in)", asMETHODPR(Point, operator-=, (const Point &), Point&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point opMul(const Point &in) const", asMETHODPR(Point, operator*, (const Point&) const, float), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point opMul(float) const", asMETHODPR(Point, operator*, (float) const, Point), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point opMul_r(float) const", asFUNCTION(pointMult), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point &opMulAssign(float)", asMETHODPR(Point, operator*=, (float), Point&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point opDiv(float) const", asMETHODPR(Point, operator/, (float) const, Point), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point &opDivAssign(float)", asMETHODPR(Point, operator/=, (float), Point&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "bool opEquals(const Point &in) const", asMETHODPR(Point, operator==, (const Point &) const, bool), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point opNeg() const", asMETHODPR(Point, operator-, () const, Point), asCALL_THISCALL); assert( r >= 0 );

		// TODO: operator[](int), set & get versions
		
		r = engine->RegisterObjectMethod("Point", "void set(float x, float y)", asMETHODPR(Point, set, (float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "void set(const Point &in)", asMETHODPR(Point, set, (const Point&), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Point", "float dot(const Point &in) const", asMETHOD(Point, dot), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "float cross(const Point &in) const", asMETHOD(Point, cross), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Point", "float norm2() const", asMETHOD(Point, norm2), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "float norm() const", asMETHOD(Point, norm), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Point", "void normalize()", asMETHOD(Point, normalize), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point normalized() const", asMETHOD(Point, normalized), asCALL_THISCALL); assert( r >= 0 );
	}

	void registerRect(asIScriptEngine* engine)
	{

	}

	void registerColor(asIScriptEngine* engine)
	{

	}

	void registerGradient(asIScriptEngine* engine)
	{

	}

	void registerTypes(asIScriptEngine* engine)
	{
		registerPoint(engine);
		registerRect(engine);
		registerColor(engine);
		registerGradient(engine);
	}

} // namespace panda
