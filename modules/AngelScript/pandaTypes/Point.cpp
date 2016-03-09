#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Types.h"

#include <panda/types/Point.h>

#include <new>

using panda::types::Point;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

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
	static void PointDefaultConstructor(Point* self)
	{ new(self) Point(); }

	static void PointCopyConstructor(const Point& other, Point* self)
	{ new(self) Point(other); }

	static void PointInitConstructor(float x, float y, Point* self)
	{ new(self) Point(x, y); }

	static void PointListConstructor(float* d, Point* self)
	{ new(self) Point(d); }

// Point wrappers
	Point PointMult(float v, const Point& p)
	{ return p * v; }

	float PointGetIndex(int idx, Point* self)
	{ if (idx == 0 || idx == 1) return (*self)[idx]; return 0.f; }

	void PointSetIndex(int idx, float value, Point* self)
	{ if(idx == 0 || idx == 1) (*self)[idx] = value; }

	int PointOpCmp(const Point& p, Point* self)
	{
		int ret = opCompare(self->x, p.x);
		if (ret != 0) return ret;
		return opCompare(self->y, p.y);
	}

	BaseDataWrapper* createPointData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Point>(input, name, help); }

	BaseDataWrapper* createPointVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Point>, panda::VectorDataWrapper<Point>>(input, name, help); }

}

namespace panda 
{

	void registerPointType(asIScriptEngine* engine)
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

	void registerPoint(asIScriptEngine* engine)
	{
		registerPointType(engine);
		registerDataType<Point>(engine, "Point");

		aatc::container::tempspec::vector<Point>::Register(engine, "Point");
		registerVectorDataType<Point>(engine, "Point");

		int r = engine->RegisterObjectMethod("PandaObject", "PointData@ createPointData(bool, const string &in, const string &in)",
			asFUNCTION(createPointData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "PointVectorData@ createPointVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createPointVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
