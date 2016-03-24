#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Path.h"
#include "Types.h"

#include <angelscript.h>
#include <cassert>

using panda::types::Path;
using panda::types::Point;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace panda
{
	class PathDataWrapper;
	class PathVectorDataWrapper;
}

namespace
{
	BaseDataWrapper* createPathData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Path, panda::PathDataWrapper>(input, name, help); }

	BaseDataWrapper* createPathVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Path>, panda::PathVectorDataWrapper>(input, name, help); }
}

namespace panda 
{

	// Path wrapper class
	class PathWrapper
	{
	public:
		using script_point_vector = aatc::container::tempspec::vector<Point>;

		static PathWrapper* factory()
		{ return new PathWrapper(); }

		static PathWrapper* create(const panda::types::Path& path)
		{ return new PathWrapper(path); }

		PathWrapper() {}
		PathWrapper(const panda::types::Path& path)
			: m_path(path) {}

		const panda::types::Path& path() const 
		{ return m_path; }

		void addRef()
		{ m_refCount++; }

		void release()
		{ if (--m_refCount == 0) delete this; }
		
		script_point_vector* getPoints() const
		{
			auto vec = new script_point_vector();
			vec->container = m_path.points;
			return vec;
		}
	
		void setPoints(const script_point_vector* vec)
		{ m_path.points = vec->container; }

		bool operator==(const PathWrapper* wrapper) const
		{ return m_path == wrapper->m_path; }

		PathWrapper& assign(const PathWrapper* other)
		{ m_path = other->m_path; return *this; }

		PathWrapper* operator+(const Point& p) const
		{ return create(m_path + p); }
		PathWrapper* operator-(const Point& p) const
		{ return create(m_path - p); }
		PathWrapper& operator+=(const Point& p)
		{ m_path += p; return *this; }
		PathWrapper& operator-=(const Point& p)
		{ m_path -= p; return *this; }

		PathWrapper* operator*(float v) const
		{ return create(m_path * v); }
		PathWrapper* operator/(float v) const
		{ return create(m_path / v); }
		PathWrapper& operator*=(float v)
		{ m_path *= v; return *this; }
		PathWrapper& operator/=(float v)
		{ m_path /= v; return *this; }

		PathWrapper* linearProduct(const Point& p) const
		{ return create(m_path.linearProduct(p)); }
		PathWrapper* linearDivision(const Point& p) const
		{ return create(m_path.linearDivision(p)); }

		PathWrapper* reversed() const
		{ return create(m_path.reversed()); }
		void reverse()
		{ m_path.reverse(); }

		PathWrapper* rotatedPath(const Point& center, float angle) const
		{ return create(rotated(m_path, center, angle)); }
		void rotatePath(const Point& center, float angle)
		{ rotate(m_path, center, angle); }

		float area()
		{ return areaOfPolygon(m_path); }

		Point centroid()
		{ return centroidOfPolygon(m_path); }

	private:
		int m_refCount = 1;
		panda::types::Path m_path;
	};

	PathWrapper* createPathWrapper(const panda::types::Path& path)
	{ return PathWrapper::create(path); }

	const panda::types::Path& getPath(const PathWrapper* wrapper)
	{ return wrapper->path(); }

	class PathDataWrapper : public BaseDataWrapper
	{
	public:
		PathDataWrapper(Data<types::Path>* data, asIScriptEngine*)
			: m_data(data) { }

		PathWrapper* getValue() const
		{ return PathWrapper::create(m_data->getValue()); }

		void setValue(const PathWrapper* wrapper)
		{ m_data->setValue(wrapper->path()); }

		int getCounter() const
		{ return m_data->getCounter(); }

	private:
		Data<types::Path>* m_data = nullptr;
	};

	class PathVectorDataWrapper : public BaseDataWrapper
	{
	public:
		using value_type = std::vector<Path>;
		using data_type = Data<value_type>;
		using script_vector = aatc::container::templated::vector;

		PathVectorDataWrapper(data_type* data, asIScriptEngine* engine)
			: m_data(data), m_engine(engine)
		{ }

		script_vector* getValue() const
		{
			auto vec = new script_vector(m_engine->GetTypeInfoByName("Path"));
			const auto& paths = m_data->getValue();
			for (const auto& path : paths)
				vec->container.push_back(PathWrapper::create(path));
			return vec;
		}
	
		void setValue(const script_vector* vec)
		{
			auto acc = m_data->getAccessor();
			acc.clear();
			acc.reserve(vec->container.size());
			for (const auto& ptr : vec->container)
				acc.push_back(static_cast<PathWrapper*>(ptr)->path());
		}

		int getCounter() const 
		{ return m_data->getCounter(); }
	
	private:
		data_type* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};


	void registerPathData(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("PathData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathData", "Path@ getValue() const",
			asMETHOD(PathDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathData", "void setValue(const Path &in)",
			asMETHOD(PathDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathData", "int getCounter()",
			asMETHOD(PathDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerPathVectorDataType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("PathVectorData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathVectorData", str("vector<Path@>@ getValue() const"),
			asMETHOD(PathVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathVectorData", str("void setValue(const vector<Path@> &in)"),
			asMETHOD(PathVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathVectorData", "int getCounter()",
			asMETHOD(PathVectorDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerPathType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("Path", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Path", asBEHAVE_FACTORY, "Path@ f()", asFUNCTION(PathWrapper::factory), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Path", asBEHAVE_ADDREF, "void f()", asMETHOD(PathWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Path", asBEHAVE_RELEASE, "void f()", asMETHOD(PathWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path& opAssign(const Path &in)", asMETHOD(PathWrapper, assign), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Path", "vector<Point>@ getPoints()", asMETHOD(PathWrapper, getPoints), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Path", "void setPoints(const vector<Point>&in)", asMETHOD(PathWrapper, setPoints), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Path", "Path@ opAdd(const Point &in) const", asMETHODPR(PathWrapper, operator+, (const Point &) const, PathWrapper*), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path& opAddAssign(const Point &in)", asMETHODPR(PathWrapper, operator+=, (const Point &), PathWrapper&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path@ opSub(const Point &in) const", asMETHODPR(PathWrapper, operator-, (const Point &) const, PathWrapper*), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path& opSubAssign(const Point &in)", asMETHODPR(PathWrapper, operator-=, (const Point &), PathWrapper&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path@ opMul(float) const", asMETHODPR(PathWrapper, operator*, (float) const, PathWrapper*), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path@ opMul_r(float) const",asMETHODPR(PathWrapper, operator*, (float) const, PathWrapper*), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path& opMulAssign(float)", asMETHODPR(PathWrapper, operator*=, (float), PathWrapper&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path@ opDiv(float) const", asMETHODPR(PathWrapper, operator/, (float) const, PathWrapper*), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "Path& opDivAssign(float)", asMETHODPR(PathWrapper, operator/=, (float), PathWrapper&), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Path", "bool opEquals(const Path &in) const", asMETHODPR(PathWrapper, operator==, (const PathWrapper*) const, bool), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Path", "Path@ linearProduct(const Point &in) const", asMETHOD(PathWrapper, linearProduct), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Path", "Path@ linearDivision(const Point &in) const", asMETHOD(PathWrapper, linearDivision), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Path", "Path@ reversed() const", asMETHOD(PathWrapper, reversed), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Path", "void reverse()", asMETHOD(PathWrapper, reverse), asCALL_THISCALL); assert(r >= 0);
		
		r = engine->RegisterObjectMethod("Path", "Path@ rotated(const Point &in, float) const", asMETHOD(PathWrapper, rotatedPath), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Path", "void rotate(const Point &in, float)", asMETHOD(PathWrapper, rotatePath), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Path", "float area() const", asMETHOD(PathWrapper, area), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Path", "Point centroid()", asMETHOD(PathWrapper, centroid), asCALL_THISCALL); assert(r >= 0);
	}

	void registerPath(asIScriptEngine* engine)
	{
		registerPathType(engine);
		registerPathData(engine);
		registerPathVectorDataType(engine);

		int r = engine->RegisterObjectMethod("PandaObject", "PathData@ createPathData(bool, const string &in, const string &in)",
			asFUNCTION(createPathData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		engine->RegisterObjectMethod("PandaObject", "PathVectorData@ createPathVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createPathVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
