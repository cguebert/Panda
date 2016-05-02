#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Path.h"
#include "Types.h"

#include <panda/types/FloatVector.h>

#include <new>

using panda::types::FloatVector;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace panda
{
	class FloatVectorDataWrapper;
	class FloatVectorVectorDataWrapper;
}

namespace
{
	BaseDataWrapper* createFloatVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<FloatVector, panda::FloatVectorDataWrapper>(name, help); }

	BaseDataWrapper* createFloatVectorVectorData(const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<FloatVector>, panda::FloatVectorVectorDataWrapper>(name, help); }
}

namespace panda 
{

	// FloatVector wrapper class
	class FloatVectorWrapper
	{
	public:
		using script_vector = aatc::container::tempspec::vector<float>;

		static FloatVectorWrapper* factory()
		{ return new FloatVectorWrapper(); }

		static FloatVectorWrapper* create(const panda::types::FloatVector& vector)
		{ return new FloatVectorWrapper(vector); }

		FloatVectorWrapper() {}
		FloatVectorWrapper(const panda::types::FloatVector& vector)
			: m_vector(vector) {}

		const panda::types::FloatVector& intVector() const 
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

		bool operator==(const FloatVectorWrapper* wrapper) const
		{ return m_vector == wrapper->m_vector; }

		FloatVectorWrapper& assign(const FloatVectorWrapper* other)
		{ m_vector = other->m_vector; return *this; }

	private:
		int m_refCount = 1;
		panda::types::FloatVector m_vector;
	};

	class FloatVectorDataWrapper : public BaseDataWrapper
	{
	public:
		FloatVectorDataWrapper(Data<types::FloatVector>* data, asIScriptEngine* engine)
			: m_data(data) { }

		FloatVectorWrapper* getValue() const
		{ return FloatVectorWrapper::create(m_data->getValue()); }

		void setValue(const FloatVectorWrapper* wrapper)
		{ m_data->setValue(wrapper->intVector()); }

		int getCounter() const 
		{ return m_data->getCounter(); }

		void setWidget(const std::string& widget)
		{ m_data->setWidget(widget); }

		void setWidgetData(const std::string& widgetData)
		{ m_data->setWidgetData(widgetData); }

	private:
		Data<types::FloatVector>* m_data = nullptr;
	};

	class FloatVectorVectorDataWrapper : public BaseDataWrapper
	{
	public:
		using value_type = std::vector<FloatVector>;
		using data_type = Data<value_type>;
		using script_vector = aatc::container::templated::vector;

		FloatVectorVectorDataWrapper(data_type* data, asIScriptEngine* engine)
			: m_data(data), m_engine(engine)
		{ }

		script_vector* getValue() const
		{
			auto vec = new script_vector(m_engine->GetTypeInfoByName("FloatVec"));
			const auto& paths = m_data->getValue();
			for (const auto& path : paths)
				vec->container.push_back(FloatVectorWrapper::create(path));
			return vec;
		}
	
		void setValue(const script_vector* vec)
		{
			auto acc = m_data->getAccessor();
			acc.clear();
			acc.reserve(vec->container.size());
			for (const auto& ptr : vec->container)
				acc.push_back(static_cast<FloatVectorWrapper*>(ptr)->intVector());
		}

		int getCounter() const 
		{ return m_data->getCounter(); }

		void setWidget(const std::string& widget)
		{ m_data->setWidget(widget); }

		void setWidgetData(const std::string& widgetData)
		{ m_data->setWidgetData(widgetData); }

	private:
		data_type* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};


	void registerFloatVectorData(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("FloatVecData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("FloatVecData", "FloatVec@ getValue() const",
			asMETHOD(FloatVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("FloatVecData", "void setValue(const FloatVec &in)",
			asMETHOD(FloatVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		registerBaseDataMethods<FloatVectorDataWrapper>(engine, "FloatVecData");
	}

	void registerFloatVectorVectorDataType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("FloatVecVectorData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("FloatVecVectorData", str("vector<FloatVec@>@ getValue() const"),
			asMETHOD(FloatVectorVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("FloatVecVectorData", str("void setValue(const vector<FloatVec@> &in)"),
			asMETHOD(FloatVectorVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		registerBaseDataMethods<FloatVectorVectorDataWrapper>(engine, "FloatVecVectorData");
	}

	void registerFloatVectorType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("FloatVec", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("FloatVec", asBEHAVE_FACTORY, "FloatVec@ f()", asFUNCTION(FloatVectorWrapper::factory), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("FloatVec", asBEHAVE_ADDREF, "void f()", asMETHOD(FloatVectorWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("FloatVec", asBEHAVE_RELEASE, "void f()", asMETHOD(FloatVectorWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("FloatVec", "FloatVec& opAssign(const FloatVec &in)", asMETHOD(FloatVectorWrapper, assign), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("FloatVec", "bool opEquals(const FloatVec &in) const", asMETHOD(FloatVectorWrapper, operator==), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod("FloatVec", "vector<float>@ getValues()", asMETHOD(FloatVectorWrapper, getValues), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("FloatVec", "void setValues(const vector<float> &in)", asMETHOD(FloatVectorWrapper, setValues), asCALL_THISCALL); assert(r >= 0);
	}

	void registerFloatVector(asIScriptEngine* engine)
	{
		registerFloatVectorType(engine);
		registerFloatVectorData(engine);
		registerFloatVectorVectorDataType(engine);

		int r = engine->RegisterObjectMethod("PandaObject", "FloatVecData@ createFloatVecData(const string &in, const string &in)",
			asFUNCTION(createFloatVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		engine->RegisterObjectMethod("PandaObject", "FloatVecVectorData@ createFloatVecVectorData(const string &in, const string &in)",
			asFUNCTION(createFloatVectorVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
