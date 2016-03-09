#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Types.h"

#include <panda/types/Mesh.h>

#include <new>

using panda::types::Point;
using panda::types::Mesh;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace panda
{
	class MeshDataWrapper;
	class MeshVectorDataWrapper;
}

namespace
{
	BaseDataWrapper* createMeshData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Mesh, panda::MeshDataWrapper>(input, name, help); }

	BaseDataWrapper* createMeshVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Mesh>, panda::MeshVectorDataWrapper>(input, name, help); }

	void EdgeDefaultConstructor(Mesh::Edge* self)
	{ new(self) Mesh::Edge({ 0, 0 }); }

	void EdgeCopyConstructor(const Mesh::Edge& other, Mesh::Edge* self)
	{ new(self) Mesh::Edge(other); }

	void EdgeInitConstructor(unsigned int p0, unsigned int p1, Mesh::Edge* self)
	{ new(self) Mesh::Edge({ p0, p1 }); }

	void TriangleDefaultConstructor(Mesh::Triangle* self)
	{ new(self) Mesh::Triangle({ 0, 0 }); }

	void TriangleCopyConstructor(const Mesh::Triangle& other, Mesh::Triangle* self)
	{ new(self) Mesh::Triangle(other); }

	void TriangleInitConstructor(unsigned int p0, unsigned int p1, unsigned int p2, Mesh::Triangle* self)
	{ new(self) Mesh::Triangle({ p0, p1, p2 }); }
}

namespace panda 
{

	// Mesh wrapper class
	class MeshWrapper
	{
	public:
		using points_vector = aatc::container::tempspec::vector<Point>;
		using edges_vector = aatc::container::tempspec::vector<Mesh::Edge>;
		using triangles_vector = aatc::container::tempspec::vector<Mesh::Triangle>;
		using script_vector = aatc::container::templated::vector;

		static void factory(asIScriptGeneric* gen)
		{ *static_cast<MeshWrapper**>(gen->GetAddressOfReturnLocation()) = new MeshWrapper(gen->GetEngine()); }

		static MeshWrapper* create(const panda::types::Mesh& mesh, asIScriptEngine* engine)
		{ return new MeshWrapper(mesh, engine); }

		MeshWrapper(asIScriptEngine* engine)
			: m_engine(engine) {}
		MeshWrapper(const panda::types::Mesh& mesh, asIScriptEngine* engine)
			: m_mesh(mesh), m_engine(engine) {}

		const panda::types::Mesh& mesh() const 
		{ return m_mesh; }

		void addRef()
		{ m_refCount++; }

		void release()
		{ if (--m_refCount == 0) delete this; }
		
		points_vector* getPoints() const
		{
			auto vec = new points_vector();
			vec->container = m_mesh.getPoints();
			return vec; 
		}
	
		void setPoints(points_vector* vec)
		{ m_mesh.clearPoints(); m_mesh.addPoints(vec->container); }

		edges_vector* getEdges() const
		{
			auto vec = new edges_vector();
			vec->container = m_mesh.getEdges();
			return vec; 
		}
	
		void setEdges(edges_vector* vec)
		{ m_mesh.clearEdges(); m_mesh.addEdges(vec->container); }

		triangles_vector* getTriangles() const
		{
			auto vec = new triangles_vector();
			vec->container = m_mesh.getTriangles();
			return vec; 
		}
	
		void setTriangles(triangles_vector* vec)
		{ m_mesh.clearTriangles(); m_mesh.addTriangles(vec->container); }

		bool operator==(const MeshWrapper* wrapper) const
		{ return m_mesh == wrapper->m_mesh; }

		static MeshWrapper& Assign(MeshWrapper* other, MeshWrapper* self)
		{ return *self = *other; }

		MeshWrapper* translatedMesh(const Point& center) const
		{ return create(translated(m_mesh, center), m_engine); }
		void translateMesh(const Point& center)
		{ translate(m_mesh, center); }

		MeshWrapper* rotatedMesh(const Point& center, float angle) const
		{ return create(rotated(m_mesh, center, angle), m_engine); }
		void rotateMesh(const Point& center, float angle)
		{ rotate(m_mesh, center, angle); }

		MeshWrapper* scaledMesh(float angle) const
		{ return create(scaled(m_mesh, angle), m_engine); }
		void scaleMesh(float angle)
		{ scale(m_mesh, angle); }

	private:
		int m_refCount = 1;
		panda::types::Mesh m_mesh;
		asIScriptEngine* m_engine = nullptr;
	};

	class MeshDataWrapper : public BaseDataWrapper
	{
	public:
		MeshDataWrapper(Data<types::Mesh>* data, asIScriptEngine* engine)
			: m_data(data), m_engine(engine) { }

		MeshWrapper* getValue() const
		{ return MeshWrapper::create(m_data->getValue(), m_engine); }

		void setValue(MeshWrapper* wrapper)
		{ m_data->setValue(wrapper->mesh()); wrapper->release(); }

		int getCounter() const
		{ return m_data->getCounter(); }

	private:
		Data<types::Mesh>* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};

	class MeshVectorDataWrapper : public BaseDataWrapper
	{
	public:
		using value_type = std::vector<Mesh>;
		using data_type = Data<value_type>;
		using script_vector = aatc::container::templated::vector;

		MeshVectorDataWrapper(data_type* data, asIScriptEngine* engine)
			: m_data(data), m_engine(engine)
		{ }

		script_vector* getValue() const
		{
			auto vec = new script_vector(m_engine->GetTypeInfoByName("Mesh"));
			const auto& paths = m_data->getValue();
			for (const auto& path : paths)
				vec->container.push_back(MeshWrapper::create(path, m_engine));
			return vec;
		}
	
		void setValue(script_vector* vec)
		{
			auto acc = m_data->getAccessor();
			acc.clear();
			acc.reserve(vec->container.size());
			for (const auto& ptr : vec->container)
				acc.push_back(static_cast<MeshWrapper*>(ptr)->mesh());
			vec->refcount_Release();
		}

		int getCounter() const 
		{ return m_data->getCounter(); }
	
	private:
		data_type* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};


	void registerMeshData(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("MeshData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshData", "Mesh@ getValue()",
			asMETHOD(MeshDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshData", "void setValue(Mesh@)",
			asMETHOD(MeshDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshData", "int getCounter()",
			asMETHOD(MeshDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerMeshVectorDataType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("MeshVectorData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshVectorData", str("vector<Mesh@>@ getValue()"),
			asMETHOD(MeshVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshVectorData", str("void setValue(vector<Mesh@>@)"),
			asMETHOD(MeshVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("MeshVectorData", "int getCounter()",
			asMETHOD(MeshVectorDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerMeshType(asIScriptEngine* engine)
	{
		int r = 0;
		// Mesh::Edge
		r = engine->RegisterObjectType("Edge", sizeof(Mesh::Edge), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLINTS | asGetTypeTraits<Mesh::Edge>()); assert(r >= 0);
		r = engine->RegisterObjectProperty("Edge", "uint p0", 0); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Edge", "uint p1", sizeof(unsigned int)); assert( r >= 0 );
		aatc::container::tempspec::vector<Mesh::Edge>::Register(engine, "Edge");

		r = engine->RegisterObjectBehaviour("Edge", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(EdgeDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Edge", asBEHAVE_CONSTRUCT, "void f(const Edge &in)", asFUNCTION(EdgeCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Edge", asBEHAVE_CONSTRUCT, "void f(uint, uint)", asFUNCTION(EdgeInitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		// Mesh::Triangle
		r = engine->RegisterObjectType("Triangle", sizeof(Mesh::Triangle), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLINTS | asGetTypeTraits<Mesh::Triangle>()); assert(r >= 0);
		r = engine->RegisterObjectProperty("Triangle", "uint p0", 0); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Triangle", "uint p1", sizeof(unsigned int)); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Triangle", "uint p2", 2 * sizeof(unsigned int)); assert( r >= 0 );
		aatc::container::tempspec::vector<Mesh::Triangle>::Register(engine, "Triangle");

		r = engine->RegisterObjectBehaviour("Triangle", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(TriangleDefaultConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Triangle", asBEHAVE_CONSTRUCT, "void f(const Triangle &in)", asFUNCTION(TriangleCopyConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Triangle", asBEHAVE_CONSTRUCT, "void f(uint, uint, uint)", asFUNCTION(TriangleInitConstructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		// Mesh
		r = engine->RegisterObjectType("Mesh", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_FACTORY, "Mesh@ f()", asFUNCTION(MeshWrapper::factory), asCALL_GENERIC); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_ADDREF, "void f()", asMETHOD(MeshWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Mesh", asBEHAVE_RELEASE, "void f()", asMETHOD(MeshWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Mesh", "Mesh& opAssign(Mesh &in)", asFUNCTION(MeshWrapper::Assign), asCALL_CDECL_OBJLAST); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Mesh", "bool opEquals(const Mesh@) const", asMETHOD(MeshWrapper, operator==), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Mesh", "vector<Point>@ getPoints()", asMETHOD(MeshWrapper, getPoints), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void setPoints(vector<Point>@)", asMETHOD(MeshWrapper, setPoints), asCALL_THISCALL); assert(r >= 0);
	
		r = engine->RegisterObjectMethod("Mesh", "vector<Edge>@ getEdges()", asMETHOD(MeshWrapper, getEdges), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void setEdges(vector<Edge>@)", asMETHOD(MeshWrapper, setEdges), asCALL_THISCALL); assert(r >= 0);
	
		r = engine->RegisterObjectMethod("Mesh", "vector<Triangle>@ getTriangles()", asMETHOD(MeshWrapper, getTriangles), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void setTriangles(vector<Triangle>@)", asMETHOD(MeshWrapper, setTriangles), asCALL_THISCALL); assert(r >= 0);
	
		r = engine->RegisterObjectMethod("Mesh", "Mesh@ translated(const Point &in) const", asMETHOD(MeshWrapper, translatedMesh), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void translate(const Point &in)", asMETHOD(MeshWrapper, translateMesh), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "Mesh@ rotated(const Point &in, float) const", asMETHOD(MeshWrapper, rotatedMesh), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void rotate(const Point &in, float)", asMETHOD(MeshWrapper, rotateMesh), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectMethod("Mesh", "Mesh@ scaled(float) const", asMETHOD(MeshWrapper, scaledMesh), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Mesh", "void scale(float)", asMETHOD(MeshWrapper, scaleMesh), asCALL_THISCALL); assert(r >= 0);
	}

	void registerMesh(asIScriptEngine* engine)
	{
		registerMeshType(engine);
		registerMeshData(engine);
		registerMeshVectorDataType(engine);

		int r = engine->RegisterObjectMethod("PandaObject", "MeshData@ createMeshData(bool, const string &in, const string &in)",
			asFUNCTION(createMeshData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		engine->RegisterObjectMethod("PandaObject", "MeshVectorData@ createMeshVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createMeshVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
