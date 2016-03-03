#include "Types.h"

#include <panda/types/Gradient.h>
#include <panda/types/Rect.h>

#include <angelscript.h>
#include <cassert>
#include <new>

namespace
{
	template <class T>
	int opCompare(const T& lhs, const T& rhs)
	{
		if (lhs < rhs) return -1;
		if (lhs > rhs) return 1;
		return 0;
	}

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
	panda::types::Point PointMult(float v, const panda::types::Point& p)
	{ return p * v; }

	float PointGetIndex(int idx, panda::types::Point* self)
	{ if (idx == 0 || idx == 1) return (*self)[idx]; return 0.f; }

	void PointSetIndex(int idx, float value, panda::types::Point* self)
	{ if(idx == 0 || idx == 1) (*self)[idx] = value; }

	int PointOpCmp(const panda::types::Point& p, panda::types::Point* self)
	{
		int ret = opCompare(self->x, p.x);
		if (ret != 0) return ret;
		return opCompare(self->y, p.y);
	}

// Rect constructors
	static void RectDefaultConstructor(panda::types::Rect* self)
	{ new(self) panda::types::Rect(); }

	static void RectCopyConstructor(const panda::types::Rect& other, panda::types::Rect* self)
	{ new(self) panda::types::Rect(other); }

	static void Rect4FloatsConstructor(float x1, float y1, float x2, float y2, panda::types::Rect* self)
	{ new(self) panda::types::Rect(x1, y1, x2, y2); }

	static void Rect2PointsConstructor(const panda::types::Point& p1, const panda::types::Point& p2, panda::types::Rect* self)
	{ new(self) panda::types::Rect(p1, p2); }

	static void RectPointSizeConstructor(const panda::types::Point& p, float w, float h, panda::types::Rect* self)
	{ new(self) panda::types::Rect(p, w, h); }

// Color constructors
	static void ColorDefaultConstructor(panda::types::Color* self)
	{ new(self) panda::types::Color(); }

	static void ColorCopyConstructor(const panda::types::Color& other, panda::types::Color* self)
	{ new(self) panda::types::Color(other); }

	static void Color4FloatsConstructor(float r, float g, float b, float a, panda::types::Color* self)
	{ new(self) panda::types::Color(r, g, b, a); }

// Color wrappers
	panda::types::Color ColorMult(float v, const panda::types::Color& p)
	{ return p * v; }

	float ColorGetIndex(int idx, panda::types::Color* self)
	{ if (idx >= 0 || idx < 4) return (*self)[idx]; return 0.f; }

	void ColorSetIndex(int idx, float value, panda::types::Color* self)
	{ if (idx >= 0 || idx < 4) (*self)[idx] = value; }
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
		r = engine->RegisterObjectMethod("Point", "Point opMul_r(float) const", asFUNCTION(PointMult), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point &opMulAssign(float)", asMETHODPR(Point, operator*=, (float), Point&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point opDiv(float) const", asMETHODPR(Point, operator/, (float) const, Point), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point &opDivAssign(float)", asMETHODPR(Point, operator/=, (float), Point&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "bool opEquals(const Point &in) const", asMETHODPR(Point, operator==, (const Point &) const, bool), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point opNeg() const", asMETHODPR(Point, operator-, () const, Point), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "int opComp(const Point &in) const", asFUNCTION(PointOpCmp), asCALL_CDECL_OBJLAST); assert(r >= 0);

		r = engine->RegisterObjectMethod("Point", "float get_opIndex(int) const", asFUNCTION(PointGetIndex), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "void set_opIndex(int, float) const", asFUNCTION(PointSetIndex), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		
		r = engine->RegisterObjectMethod("Point", "void set(float, float)", asMETHODPR(Point, set, (float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "void set(const Point &in)", asMETHODPR(Point, set, (const Point&), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Point", "Point linearProduct(const Point &in) const", asMETHOD(Point, linearProduct), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point linearDivision(const Point &in) const", asMETHOD(Point, linearDivision), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Point", "float dot(const Point &in) const", asMETHOD(Point, dot), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "float cross(const Point &in) const", asMETHOD(Point, cross), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Point", "float norm2() const", asMETHOD(Point, norm2), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "float norm() const", asMETHOD(Point, norm), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Point", "void normalize()", asMETHOD(Point, normalize), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Point", "Point normalized() const", asMETHOD(Point, normalized), asCALL_THISCALL); assert( r >= 0 );
	}

	void registerRect(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("Rect", sizeof(Rect), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS  | asGetTypeTraits<Rect>()); assert(r >= 0);
		r = engine->RegisterObjectProperty("Rect", "float x1", asOFFSET(Rect, x1)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Rect", "float y1", asOFFSET(Rect, y1)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Rect", "float x2", asOFFSET(Rect, x2)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Rect", "float y2", asOFFSET(Rect, y2)); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(RectDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(Rect4FloatsConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(const Rect &in)", asFUNCTION(RectCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(const Point &in, const Point &in)", asFUNCTION(Rect2PointsConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(const Point &in, float, float)", asFUNCTION(RectPointSizeConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void set(float, float, float, float)", asMETHODPR(Rect, set, (float, float, float, float), void), asCALL_THISCALL); assert( r >= 0 );
		
		r = engine->RegisterObjectMethod("Rect", "void normalize()", asMETHOD(Rect, canonicalize), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect normalized() const", asMETHOD(Rect, canonicalized), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "float width() const", asMETHOD(Rect, width), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float height() const", asMETHOD(Rect, height), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point size() const", asMETHOD(Rect, size), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float aspectRatio() const", asMETHOD(Rect, aspectRatio), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float area() const", asMETHOD(Rect, area), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "bool empty() const", asMETHOD(Rect, empty), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void setWidth(float)", asMETHOD(Rect, setWidth), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setHeight(float)", asMETHOD(Rect, setHeight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setSize(float, float)", asMETHODPR(Rect, setSize, (float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setSize(const Point &in)", asMETHODPR(Rect, setSize, (const Point&), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "float left() const", asMETHOD(Rect, left), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float top() const", asMETHOD(Rect, top), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float right() const", asMETHOD(Rect, right), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float bottom() const", asMETHOD(Rect, bottom), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void setLeft(float)", asMETHOD(Rect, setLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setTop(float)", asMETHOD(Rect, setTop), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setRight(float)", asMETHOD(Rect, setRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setBottom(float)", asMETHOD(Rect, setBottom), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "Point topLeft() const", asMETHOD(Rect, topLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point bottomRight() const", asMETHOD(Rect, bottomRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point topRight() const", asMETHOD(Rect, topRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point bottomLeft() const", asMETHOD(Rect, bottomLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point center() const", asMETHOD(Rect, center), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void setTopLeft(const Point &in)", asMETHOD(Rect, setTopLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setBottomRight(const Point &in)", asMETHOD(Rect, setBottomRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setTopRight(const Point &in)", asMETHOD(Rect, setTopRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setBottomLeft(const Point &in)", asMETHOD(Rect, setBottomLeft), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void translate(const Point &in)", asMETHODPR(Rect, translate, (const Point&), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void translate(float, float)", asMETHODPR(Rect, translate, (float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect translated(const Point &in) const", asMETHODPR(Rect, translated, (const Point&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect translated(float, float) const", asMETHODPR(Rect, translated, (float, float) const, Rect), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void moveTo(const Point &in)", asMETHODPR(Rect, moveTo, (const Point&), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveTo(float, float)", asMETHODPR(Rect, moveTo, (float, float), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void moveLeft(float)", asMETHOD(Rect, moveLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveTop(float)", asMETHOD(Rect, moveTop), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveRight(float)", asMETHOD(Rect, moveRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveBottom(float)", asMETHOD(Rect, moveBottom), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void moveTopLeft(const Point &in)", asMETHOD(Rect, moveTopLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveBottomRight(const Point &in)", asMETHOD(Rect, moveBottomRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveTopRight(const Point &in)", asMETHOD(Rect, moveTopRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveBottomLeft(const Point &in)", asMETHOD(Rect, moveBottomLeft), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void moveCenter(const Point &in)", asMETHOD(Rect, moveCenter), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void adjust(float, float, float, float)", asMETHODPR(Rect, adjust, (float, float, float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void adjust(const Rect &in)", asMETHODPR(Rect, adjust, (const Rect&), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "Rect adjusted(float, float, float, float) const", asMETHODPR(Rect, adjusted, (float, float, float, float) const, Rect), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect adjusted(const Rect &in) const", asMETHODPR(Rect, adjusted, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "Rect &opOrAssign(const Rect &in)", asMETHODPR(Rect, operator|=, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect opOr(const Rect &in) const", asMETHODPR(Rect, operator|, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect united(const Rect &in) const", asMETHOD(Rect, united), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "Rect &opAndAssign(const Rect &in)", asMETHODPR(Rect, operator&=, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect opAnd(const Rect &in) const", asMETHODPR(Rect, operator&, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect intersected(const Rect &in) const", asMETHOD(Rect, intersected), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "bool intersects(const Rect &in) const", asMETHOD(Rect, intersects), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "bool contains(const Rect &in) const", asMETHODPR(Rect, contains, (const Rect&) const, bool), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "bool contains(const Point &in) const", asMETHODPR(Rect, contains, (const Point&) const, bool), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "bool contains(float, float) const", asMETHODPR(Rect, contains, (float, float) const, bool), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "bool opEquals(const Rect &in) const", asMETHODPR(Rect, operator==, (const Rect &) const, bool), asCALL_THISCALL); assert( r >= 0 );
	}

	void registerColor(asIScriptEngine* engine)
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

	void registerGradient(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("Gradient", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Gradient", asBEHAVE_FACTORY, "Gradient@ f()", asFUNCTION(GradientWrapper::factory), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Gradient", asBEHAVE_ADDREF, "void f()", asMETHOD(GradientWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Gradient", asBEHAVE_RELEASE, "void f()", asMETHOD(GradientWrapper, release), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Gradient", "void clear()", asMETHOD(GradientWrapper, clear), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Gradient", "void add(float, Color)", asMETHOD(GradientWrapper, add), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Gradient", "Color get(float)", asMETHOD(GradientWrapper, get), asCALL_THISCALL); assert( r >= 0 );
	}

	void registerTypes(asIScriptEngine* engine)
	{
		registerPoint(engine);
		registerRect(engine);
		registerColor(engine);
		registerGradient(engine);
	}

} // namespace panda
