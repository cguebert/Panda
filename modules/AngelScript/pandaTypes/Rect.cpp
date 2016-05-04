#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Types.h"

#include <panda/types/Rect.h>

#include <new>

using panda::types::Point;
using panda::types::Rect;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace
{

// Rect constructors
	void RectDefaultConstructor(Rect* self)
	{ new(self) Rect(); }

	void RectCopyConstructor(const Rect& other, Rect* self)
	{ new(self) Rect(other); }

	void Rect4FloatsConstructor(float x1, float y1, float x2, float y2, Rect* self)
	{ new(self) Rect(x1, y1, x2, y2); }

	void Rect2PointsConstructor(const Point& p1, const Point& p2, Rect* self)
	{ new(self) Rect(p1, p2); }

	void RectPointSizeConstructor(const Point& p, float w, float h, Rect* self)
	{ new(self) Rect(p, w, h); }

	BaseDataWrapper* createRectData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Rect>(name, help); }

	BaseDataWrapper* createRectVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Rect>, panda::VectorDataWrapper<Rect>>(name, help); }

}

namespace panda 
{

	void registerRectType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("Rect", sizeof(Rect), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS  | asGetTypeTraits<Rect>()); assert(r >= 0);
		r = engine->RegisterObjectProperty("Rect", "float x1", asOFFSET(Rect, x1)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Rect", "float y1", asOFFSET(Rect, y1)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Rect", "float x2", asOFFSET(Rect, x2)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Rect", "float y2", asOFFSET(Rect, y2)); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(RectDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(float left, float top, float right, float bottom)", asFUNCTION(Rect4FloatsConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(const Rect &in rect)", asFUNCTION(RectCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(const Point &in topLeft, const Point &in bottomRight)", asFUNCTION(Rect2PointsConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Rect", asBEHAVE_CONSTRUCT, "void f(const Point &in topLeft, float width, float height)", asFUNCTION(RectPointSizeConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void set(float left, float top, float right, float bottom)", asMETHODPR(Rect, set, (float, float, float, float), void), asCALL_THISCALL); assert( r >= 0 );
		
		r = engine->RegisterObjectMethod("Rect", "void normalize()", asMETHOD(Rect, canonicalize), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect normalized() const", asMETHOD(Rect, canonicalized), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "float width() const", asMETHOD(Rect, width), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float height() const", asMETHOD(Rect, height), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point size() const", asMETHOD(Rect, size), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float aspectRatio() const", asMETHOD(Rect, aspectRatio), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float area() const", asMETHOD(Rect, area), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "bool empty() const", asMETHOD(Rect, empty), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void setWidth(float width)", asMETHOD(Rect, setWidth), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setHeight(float height)", asMETHOD(Rect, setHeight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setSize(float width, float height)", asMETHODPR(Rect, setSize, (float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setSize(const Point &in size)", asMETHODPR(Rect, setSize, (const Point&), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "float left() const", asMETHOD(Rect, left), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float top() const", asMETHOD(Rect, top), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float right() const", asMETHOD(Rect, right), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "float bottom() const", asMETHOD(Rect, bottom), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void setLeft(float left)", asMETHOD(Rect, setLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setTop(float top)", asMETHOD(Rect, setTop), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setRight(float right)", asMETHOD(Rect, setRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setBottom(float bottom)", asMETHOD(Rect, setBottom), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "Point topLeft() const", asMETHOD(Rect, topLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point bottomRight() const", asMETHOD(Rect, bottomRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point topRight() const", asMETHOD(Rect, topRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point bottomLeft() const", asMETHOD(Rect, bottomLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Point center() const", asMETHOD(Rect, center), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void setTopLeft(const Point &in topLeft)", asMETHOD(Rect, setTopLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setBottomRight(const Point &in bottomRight)", asMETHOD(Rect, setBottomRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setTopRight(const Point &in topRight)", asMETHOD(Rect, setTopRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void setBottomLeft(const Point &in bottomLeft)", asMETHOD(Rect, setBottomLeft), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void translate(const Point &in delta)", asMETHODPR(Rect, translate, (const Point&), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void translate(float dx, float dy)", asMETHODPR(Rect, translate, (float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect translated(const Point &in delta) const", asMETHODPR(Rect, translated, (const Point&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect translated(float dx, float dy) const", asMETHODPR(Rect, translated, (float, float) const, Rect), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void moveTo(const Point &in topLeft)", asMETHODPR(Rect, moveTo, (const Point&), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveTo(float left, float top)", asMETHODPR(Rect, moveTo, (float, float), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void moveLeft(float left)", asMETHOD(Rect, moveLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveTop(float top)", asMETHOD(Rect, moveTop), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveRight(float right)", asMETHOD(Rect, moveRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveBottom(float bottom)", asMETHOD(Rect, moveBottom), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void moveTopLeft(const Point &in topLeft)", asMETHOD(Rect, moveTopLeft), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveBottomRight(const Point &in bottomRight)", asMETHOD(Rect, moveBottomRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveTopRight(const Point &in topRight)", asMETHOD(Rect, moveTopRight), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void moveBottomLeft(const Point &in bottomLeft)", asMETHOD(Rect, moveBottomLeft), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void moveCenter(const Point &in center)", asMETHOD(Rect, moveCenter), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "void adjust(float left, float top, float right, float bottom)", asMETHODPR(Rect, adjust, (float, float, float, float), void), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "void adjust(const Rect &in rect)", asMETHODPR(Rect, adjust, (const Rect&), void), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "Rect adjusted(float left, float top, float right, float bottom) const", asMETHODPR(Rect, adjusted, (float, float, float, float) const, Rect), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect adjusted(const Rect &in rect) const", asMETHODPR(Rect, adjusted, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "Rect &opOrAssign(const Rect &in rect)", asMETHODPR(Rect, operator|=, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect opOr(const Rect &in rect) const", asMETHODPR(Rect, operator|, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect united(const Rect &in rect) const", asMETHOD(Rect, united), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "Rect &opAndAssign(const Rect &in rect)", asMETHODPR(Rect, operator&=, (const Rect&), Rect&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect opAnd(const Rect &in rect) const", asMETHODPR(Rect, operator&, (const Rect&) const, Rect), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "Rect intersected(const Rect &in rect) const", asMETHOD(Rect, intersected), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "bool intersects(const Rect &in rect) const", asMETHOD(Rect, intersects), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "bool contains(const Rect &in rect) const", asMETHODPR(Rect, contains, (const Rect&) const, bool), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "bool contains(const Point &in point) const", asMETHODPR(Rect, contains, (const Point&) const, bool), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Rect", "bool contains(float px, float py) const", asMETHODPR(Rect, contains, (float, float) const, bool), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Rect", "bool opEquals(const Rect &in rect) const", asMETHODPR(Rect, operator==, (const Rect &) const, bool), asCALL_THISCALL); assert( r >= 0 );
	}

	void registerRect(asIScriptEngine* engine)
	{
		registerRectType(engine);
		registerDataType<Rect>(engine, "Rect");

		aatc::container::tempspec::vector<Rect>::Register(engine, "Rect");
		registerVectorDataType<Rect>(engine, "Rect");

		int r = engine->RegisterObjectMethod("PandaObject", "RectData@ createRectData(const string &in name, const string &in description)",
			asFUNCTION(createRectData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "RectVectorData@ createRectVectorData(const string &in name, const string &in description)",
			asFUNCTION(createRectVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
