#include "Types.h"
#include "DataWrapper.h"
#include "ObjectWrapper.h"

#include <panda/types/Path.h>

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
			auto* vec = new script_point_vector();
			vec->container = m_path.points;
			return vec;
		}
	
		void setPoints(script_point_vector* vec)
		{ m_path.points = vec->container; }

		bool operator==(const PathWrapper& wrapper) const
		{ return m_path == wrapper.m_path; }

		static PathWrapper& Assign(PathWrapper* other, PathWrapper* self)
		{ return *self = *other; }

	private:
		int m_refCount = 1;
		panda::types::Path m_path;
	};

	class PathDataWrapper : public BaseDataWrapper
	{
	public:
		PathDataWrapper(Data<types::Path>* data, asIScriptEngine*)
			: m_data(data) { }

		PathWrapper* getValue() const
		{ return PathWrapper::create(m_data->getValue()); }

		void setValue(PathWrapper* wrapper)
		{ m_data->setValue(wrapper->path()); wrapper->release(); }

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
			auto* vec = new script_vector(m_engine->GetTypeInfoByName("Path"));
			const auto& paths = m_data->getValue();
			for (const auto& path : paths)
				vec->container.push_back(PathWrapper::create(path));
			return vec;
		}
	
		void setValue(script_vector* vec)
		{
			auto acc = m_data->getAccessor();
			acc.clear();
			acc.reserve(vec->container.size());
			for (const auto& ptr : vec->container)
				acc.push_back(static_cast<PathWrapper*>(ptr)->path());
			vec->refcount_Release();
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
		r = engine->RegisterObjectMethod("PathData", "Path@ getValue()",
			asMETHOD(PathDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathData", "void setValue(Path@)",
			asMETHOD(PathDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathData", "int getCounter()",
			asMETHOD(PathDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerPathVectorDataType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("PathVectorData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathVectorData", str("vector<Path@>@ getValue()"),
			asMETHOD(PathVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PathVectorData", str("void setValue(vector<Path@>@)"),
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
		r = engine->RegisterObjectMethod("Path", "Path& opAssign(Path &in)", asFUNCTION(PathWrapper::Assign), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Path", "vector<Point>@ getPoints()", asMETHOD(PathWrapper, getPoints), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Path", "void setPoints(vector<Point>@)", asMETHOD(PathWrapper, setPoints), asCALL_THISCALL); assert(r >= 0);
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
