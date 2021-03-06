#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Path.h"
#include "Types.h"

#include <panda/types/Polygon.h>

#include <new>

using panda::types::Path;
using panda::types::Point;
using panda::types::Polygon;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace panda
{
	class PolygonDataWrapper;
	class PolygonVectorDataWrapper;
}

namespace
{
	BaseDataWrapper* createPolygonData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Polygon, panda::PolygonDataWrapper>(name, help); }

	BaseDataWrapper* createPolygonVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Polygon>, panda::PolygonVectorDataWrapper>(name, help); }
}

namespace panda 
{

	// Polygon wrapper class
	class PolygonWrapper
	{
	public:
		using script_vector = aatc::container::templated::vector;

		static void factory(asIScriptGeneric* gen)
		{ *static_cast<PolygonWrapper**>(gen->GetAddressOfReturnLocation()) = new PolygonWrapper(gen->GetEngine()); }

		static PolygonWrapper* create(const panda::types::Polygon& polygon, asIScriptEngine* engine)
		{ return new PolygonWrapper(polygon, engine); }

		PolygonWrapper(asIScriptEngine* engine)
			: m_engine(engine) {}
		PolygonWrapper(const panda::types::Polygon& polygon, asIScriptEngine* engine)
			: m_polygon(polygon), m_engine(engine) {}

		const panda::types::Polygon& polygon() const 
		{ return m_polygon; }

		void addRef()
		{ m_refCount++; }

		void release()
		{ if (--m_refCount == 0) delete this; }
		
		PathWrapper* getContour() const
		{ return createPathWrapper(m_polygon.contour); }
	
		void setContour(const PathWrapper* wrapper)
		{ m_polygon.contour = getPath(wrapper); }

		script_vector* getHoles() const
		{
			auto vec = new script_vector(m_engine->GetTypeInfoByName("Path"));
			for (const auto& path : m_polygon.holes)
				vec->container.push_back(createPathWrapper(path));
			return vec;
		}
	
		void setHoles(const script_vector* vec)
		{
			m_polygon.holes.clear();
			for (const auto ptr : vec->container)
				m_polygon.holes.push_back(getPath(static_cast<PathWrapper*>(ptr)));
		}

		bool operator==(const PolygonWrapper* wrapper) const
		{ return m_polygon == wrapper->m_polygon; }

		PolygonWrapper& assign(const PolygonWrapper* other)
		{ m_polygon = other->m_polygon; return *this; }

		PolygonWrapper* translatedPolygon(const Point& center) const
		{ return create(translated(m_polygon, center), m_engine); }
		void translatePolygon(const Point& center)
		{ translate(m_polygon, center); }

		PolygonWrapper* rotatedPolygon(const Point& center, float angle) const
		{ return create(rotated(m_polygon, center, angle), m_engine); }
		void rotatePolygon(const Point& center, float angle)
		{ rotate(m_polygon, center, angle); }

		PolygonWrapper* scaledPolygon(float angle) const
		{ return create(scaled(m_polygon, angle), m_engine); }
		void scalePolygon(float angle)
		{ scale(m_polygon, angle); }

	private:
		int m_refCount = 1;
		panda::types::Polygon m_polygon;
		asIScriptEngine* m_engine = nullptr;
	};

	class PolygonDataWrapper : public BaseDataWrapper
	{
	public:
		PolygonDataWrapper(Data<types::Polygon>* data, asIScriptEngine* engine)
			: BaseDataWrapper(data), m_data(data), m_engine(engine) { }

		PolygonWrapper* getValue() const
		{ return PolygonWrapper::create(m_data->getValue(), m_engine); }

		void setValue(const PolygonWrapper* wrapper)
		{ m_data->setValue(wrapper->polygon()); }

	private:
		Data<types::Polygon>* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};

	class PolygonVectorDataWrapper : public BaseDataWrapper
	{
	public:
		using value_type = std::vector<Polygon>;
		using data_type = Data<value_type>;
		using script_vector = aatc::container::templated::vector;

		PolygonVectorDataWrapper(data_type* data, asIScriptEngine* engine)
			: BaseDataWrapper(data), m_data(data), m_engine(engine)
		{ }

		script_vector* getValue() const
		{
			auto vec = new script_vector(m_engine->GetTypeInfoByName("Polygon"));
			const auto& paths = m_data->getValue();
			for (const auto& path : paths)
				vec->container.push_back(PolygonWrapper::create(path, m_engine));
			return vec;
		}
	
		void setValue(const script_vector* vec)
		{
			auto acc = m_data->getAccessor();
			acc.clear();
			acc.reserve(vec->container.size());
			for (const auto& ptr : vec->container)
				acc.push_back(static_cast<PolygonWrapper*>(ptr)->polygon());
		}

	private:
		data_type* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};


	void registerPolygonData(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("PolygonData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("PolygonData", "Polygon@ getValue() const",
			asMETHOD(PolygonDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PolygonData", "void setValue(const Polygon &in value)",
			asMETHOD(PolygonDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		registerBaseDataMethods<PolygonDataWrapper>(engine, "PolygonData");
	}

	void registerPolygonVectorDataType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("PolygonVectorData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("PolygonVectorData", str("vector<Polygon@>@ getValue() const"),
			asMETHOD(PolygonVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("PolygonVectorData", str("void setValue(const vector<Polygon@> &in value)"),
			asMETHOD(PolygonVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		registerBaseDataMethods<PolygonVectorDataWrapper>(engine, "PolygonVectorData");
	}

	void registerPolygonType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("Polygon", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Polygon", asBEHAVE_FACTORY, "Polygon@ f()", asFUNCTION(PolygonWrapper::factory), asCALL_GENERIC); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Polygon", asBEHAVE_ADDREF, "void f()", asMETHOD(PolygonWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Polygon", asBEHAVE_RELEASE, "void f()", asMETHOD(PolygonWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Polygon", "Polygon& opAssign(const Polygon &in poly)", asMETHOD(PolygonWrapper, assign), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Polygon", "bool opEquals(const Polygon &in poly) const", asMETHOD(PolygonWrapper, operator==), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Polygon", "Path@ getContour()", asMETHOD(PolygonWrapper, getContour), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Polygon", "void setContour(const Path &in contour)", asMETHOD(PolygonWrapper, setContour), asCALL_THISCALL); assert(r >= 0);
	
		r = engine->RegisterObjectMethod("Polygon", "vector<Path@>@ getHoles()", asMETHOD(PolygonWrapper, getHoles), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Polygon", "void setHoles(const vector<Path@> &in holes)", asMETHOD(PolygonWrapper, setHoles), asCALL_THISCALL); assert(r >= 0);
	
		r = engine->RegisterObjectMethod("Polygon", "Polygon@ translated(const Point &in delta) const", asMETHOD(PolygonWrapper, translatedPolygon), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Polygon", "void translate(const Point &in delta)", asMETHOD(PolygonWrapper, translatePolygon), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Polygon", "Polygon@ rotated(const Point &in center, float angle) const", asMETHOD(PolygonWrapper, rotatedPolygon), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Polygon", "void rotate(const Point &in center, float angle)", asMETHOD(PolygonWrapper, rotatePolygon), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Polygon", "Polygon@ scaled(float scale) const", asMETHOD(PolygonWrapper, scaledPolygon), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Polygon", "void scale(float scale)", asMETHOD(PolygonWrapper, scalePolygon), asCALL_THISCALL); assert(r >= 0);
	}

	void registerPolygon(asIScriptEngine* engine)
	{
		registerPolygonType(engine);
		registerPolygonData(engine);
		registerPolygonVectorDataType(engine);

		int r = engine->RegisterObjectMethod("PandaObject", "PolygonData@ createPolygonData(const string &in name, const string &in description)",
			asFUNCTION(createPolygonData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		engine->RegisterObjectMethod("PandaObject", "PolygonVectorData@ createPolygonVectorData(const string &in name, const string &in description)",
			asFUNCTION(createPolygonVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
