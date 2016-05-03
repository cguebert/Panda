#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Path.h"
#include "Types.h"

#include <panda/types/IntVector.h>

#include <new>

using panda::types::IntVector;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace panda
{
	class IntVectorDataWrapper;
	class IntVectorVectorDataWrapper;
}

namespace
{
	BaseDataWrapper* createIntVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<IntVector, panda::IntVectorDataWrapper>(name, help); }

	BaseDataWrapper* createIntVectorVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<IntVector>, panda::IntVectorVectorDataWrapper>(name, help); }
}

namespace panda 
{

	// IntVector wrapper class
	class IntVectorWrapper
	{
	public:
		using script_vector = aatc::container::tempspec::vector<int>;

		static IntVectorWrapper* factory()
		{ return new IntVectorWrapper(); }

		static IntVectorWrapper* create(const panda::types::IntVector& vector)
		{ return new IntVectorWrapper(vector); }

		IntVectorWrapper() {}
		IntVectorWrapper(const panda::types::IntVector& vector)
			: m_vector(vector) {}

		const panda::types::IntVector& intVector() const 
		{ return m_vector; }

		void addRef()
		{ m_refCount++; }

		void release()
		{ if (--m_refCount == 0) delete this; }

		script_vector* getValues() const
		{
			auto vec = new script_vector();
			vec->container = m_vector.values;
			return vec;
		}
	
		void setValues(const script_vector* vec)
		{ m_vector.values = vec->container; }

		bool operator==(const IntVectorWrapper* wrapper) const
		{ return m_vector == wrapper->m_vector; }

		IntVectorWrapper& assign(const IntVectorWrapper* other)
		{ m_vector = other->m_vector; return *this; }

	private:
		int m_refCount = 1;
		panda::types::IntVector m_vector;
	};

	class IntVectorDataWrapper : public BaseDataWrapper
	{
	public:
		IntVectorDataWrapper(Data<types::IntVector>* data, asIScriptEngine* engine)
			: BaseDataWrapper(data), m_data(data) { }

		IntVectorWrapper* getValue() const
		{ return IntVectorWrapper::create(m_data->getValue()); }

		void setValue(const IntVectorWrapper* wrapper)
		{ m_data->setValue(wrapper->intVector()); }

	private:
		Data<types::IntVector>* m_data = nullptr;
	};

	class IntVectorVectorDataWrapper : public BaseDataWrapper
	{
	public:
		using value_type = std::vector<IntVector>;
		using data_type = Data<value_type>;
		using script_vector = aatc::container::templated::vector;

		IntVectorVectorDataWrapper(data_type* data, asIScriptEngine* engine)
			: BaseDataWrapper(data), m_data(data), m_engine(engine)
		{ }

		script_vector* getValue() const
		{
			auto vec = new script_vector(m_engine->GetTypeInfoByName("IntVec"));
			const auto& paths = m_data->getValue();
			for (const auto& path : paths)
				vec->container.push_back(IntVectorWrapper::create(path));
			return vec;
		}
	
		void setValue(const script_vector* vec)
		{
			auto acc = m_data->getAccessor();
			acc.clear();
			acc.reserve(vec->container.size());
			for (const auto& ptr : vec->container)
				acc.push_back(static_cast<IntVectorWrapper*>(ptr)->intVector());
		}

	private:
		data_type* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};


	void registerIntVectorData(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("IntVecData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("IntVecData", "IntVec@ getValue() const",
			asMETHOD(IntVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("IntVecData", "void setValue(const IntVec &in)",
			asMETHOD(IntVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		registerBaseDataMethods<IntVectorDataWrapper>(engine, "IntVecData");
	}

	void registerIntVectorVectorDataType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("IntVecVectorData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("IntVecVectorData", str("vector<IntVec@>@ getValue() const"),
			asMETHOD(IntVectorVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("IntVecVectorData", str("void setValue(const vector<IntVec@> &in)"),
			asMETHOD(IntVectorVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		registerBaseDataMethods<IntVectorVectorDataWrapper>(engine, "IntVecVectorData");
	}

	void registerIntVectorType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("IntVec", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("IntVec", asBEHAVE_FACTORY, "IntVec@ f()", asFUNCTION(IntVectorWrapper::factory), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("IntVec", asBEHAVE_ADDREF, "void f()", asMETHOD(IntVectorWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("IntVec", asBEHAVE_RELEASE, "void f()", asMETHOD(IntVectorWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("IntVec", "IntVec& opAssign(const IntVec &in)", asMETHOD(IntVectorWrapper, assign), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("IntVec", "bool opEquals(const IntVec &in) const", asMETHOD(IntVectorWrapper, operator==), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("IntVec", "vector<int>@ getValues()", asMETHOD(IntVectorWrapper, getValues), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("IntVec", "void setValues(const vector<int> &in)", asMETHOD(IntVectorWrapper, setValues), asCALL_THISCALL); assert(r >= 0);
	}

	void registerIntVector(asIScriptEngine* engine)
	{
		registerIntVectorType(engine);
		registerIntVectorData(engine);
		registerIntVectorVectorDataType(engine);

		int r = engine->RegisterObjectMethod("PandaObject", "IntVecData@ createIntVecData(const string &in, const string &in)",
			asFUNCTION(createIntVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		engine->RegisterObjectMethod("PandaObject", "IntVecVectorData@ createIntVecVectorData(const string &in, const string &in)",
			asFUNCTION(createIntVectorVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
