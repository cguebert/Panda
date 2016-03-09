#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Types.h"

#include <panda/types/Gradient.h>

#include <new>

using panda::types::Gradient;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace panda
{
	class GradientDataWrapper;
	class GradientVectorDataWrapper;
}

namespace
{
	BaseDataWrapper* createGradientData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Gradient, panda::GradientDataWrapper>(input, name, help); }

	BaseDataWrapper* createGradientVectorData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<std::vector<Gradient>, panda::GradientVectorDataWrapper>(input, name, help); }
}

namespace panda 
{

	// Gradient wrapper class
	class GradientWrapper
	{
	public:
		static GradientWrapper* factory()
		{ return new GradientWrapper(); }

		static GradientWrapper* create(const panda::types::Gradient& gradient)
		{ return new GradientWrapper(gradient); }

		GradientWrapper() {}
		GradientWrapper(const panda::types::Gradient& gradient)
			: m_gradient(gradient) {}

		const panda::types::Gradient& gradient() const 
		{ return m_gradient; }

		void addRef()
		{ m_refCount++; }

		void release()
		{ if (--m_refCount == 0) delete this; }

		void clear() 
		{ m_gradient.clear(); }

		void add(float pos, panda::types::Color col) 
		{ m_gradient.add(pos, col); }

		panda::types::Color get(float pos)
		{ return m_gradient.get(pos); }

		bool operator==(const GradientWrapper& wrapper) const
		{ return m_gradient == wrapper.m_gradient; }

		static GradientWrapper& Assign(GradientWrapper* other, GradientWrapper* self)
		{ return *self = *other; }

	private:
		panda::types::Gradient m_gradient;
		int m_refCount = 1;
	};

	class GradientDataWrapper : public BaseDataWrapper
	{
	public:
		GradientDataWrapper(Data<types::Gradient>* data, asIScriptEngine*)
			: m_data(data) { }

		GradientWrapper* getValue() const
		{ return GradientWrapper::create(m_data->getValue()); }

		void setValue(GradientWrapper* wrapper)
		{ m_data->setValue(wrapper->gradient()); wrapper->release(); }

		int getCounter() const
		{ return m_data->getCounter(); }

	private:
		Data<types::Gradient>* m_data = nullptr;
	};

	class GradientVectorDataWrapper : public BaseDataWrapper
	{
	public:
		using value_type = std::vector<Gradient>;
		using data_type = Data<value_type>;
		using script_vector = aatc::container::templated::vector;

		GradientVectorDataWrapper(data_type* data, asIScriptEngine* engine)
			: m_data(data), m_engine(engine)
		{ }

		script_vector* getValue() const
		{
			auto* vec = new script_vector(m_engine->GetTypeInfoByName("Gradient"));
			const auto& gradients = m_data->getValue();
			for (const auto& gradient : gradients)
				vec->container.push_back(GradientWrapper::create(gradient));
			return vec;
		}
	
		void setValue(script_vector* vec)
		{
			auto acc = m_data->getAccessor();
			acc.clear();
			acc.reserve(vec->container.size());
			for (const auto& ptr : vec->container)
				acc.push_back(static_cast<GradientWrapper*>(ptr)->gradient());
			vec->refcount_Release();
		}

		int getCounter() const 
		{ return m_data->getCounter(); }
	
	private:
		data_type* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};


	void registerGradientData(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("GradientData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("GradientData", "Gradient@ getValue()",
			asMETHOD(GradientDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("GradientData", "void setValue(Gradient@)",
			asMETHOD(GradientDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("GradientData", "int getCounter()",
			asMETHOD(GradientDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerGradientVectorDataType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("GradientVectorData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("GradientVectorData", str("vector<Gradient@>@ getValue()"),
			asMETHOD(GradientVectorDataWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("GradientVectorData", str("void setValue(vector<Gradient@>@)"),
			asMETHOD(GradientVectorDataWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("GradientVectorData", "int getCounter()",
			asMETHOD(GradientVectorDataWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerGradientType(asIScriptEngine* engine)
	{
		int r = 0;
		r = engine->RegisterObjectType("Gradient", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Gradient", asBEHAVE_FACTORY, "Gradient@ f()", asFUNCTION(GradientWrapper::factory), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour("Gradient", asBEHAVE_ADDREF, "void f()", asMETHOD(GradientWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("Gradient", asBEHAVE_RELEASE, "void f()", asMETHOD(GradientWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Gradient", "Gradient& opAssign(Gradient &in)", asFUNCTION(GradientWrapper::Assign), asCALL_CDECL_OBJLAST); assert( r >= 0 );

		r = engine->RegisterObjectMethod("Gradient", "void clear()", asMETHOD(GradientWrapper, clear), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Gradient", "void add(float, Color)", asMETHOD(GradientWrapper, add), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("Gradient", "Color get(float)", asMETHOD(GradientWrapper, get), asCALL_THISCALL); assert( r >= 0 );
	}

	void registerGradient(asIScriptEngine* engine)
	{
		registerGradientType(engine);
		registerGradientData(engine);
		registerGradientVectorDataType(engine);

		int r = engine->RegisterObjectMethod("PandaObject", "GradientData@ createGradientData(bool, const string &in, const string &in)",
			asFUNCTION(createGradientData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		engine->RegisterObjectMethod("PandaObject", "GradientVectorData@ createGradientVectorData(bool, const string &in, const string &in)",
			asFUNCTION(createGradientVectorData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
