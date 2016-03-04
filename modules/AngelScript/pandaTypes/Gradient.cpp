#include "Types.h"
#include "DataWrapper.h"
#include "ObjectWrapper.h"

#include <panda/types/Gradient.h>
#include <panda/types/Rect.h>

#include <angelscript.h>
#include <cassert>
#include <new>

using panda::types::Gradient;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace
{
	BaseDataWrapper* createGradientData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Gradient>(input, name, help); }
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

		void addRef() { m_refCount++; }
		void release() { if (--m_refCount == 0) delete this; }

		void clear() { m_gradient.clear(); }
		void add(float pos, panda::types::Color col) { m_gradient.add(pos, col); }
		panda::types::Color get(float pos) { return m_gradient.get(pos); }

	private:
		panda::types::Gradient m_gradient;
		int m_refCount = 1;
	};

	template <>
	class DataWrapper<types::Gradient, GradientWrapper*> : public BaseDataWrapper
	{
	public:
		DataWrapper(Data<types::Gradient>* data)
			: m_data(data) { }

		GradientWrapper* getValue() const
		{ return GradientWrapper::create(m_data->getValue()); }

		void setValue(GradientWrapper* wrapper)
		{ m_data->setValue(wrapper->gradient()); }

		int getCounter() const
		{ return m_data->getCounter(); }

	private:
		Data<types::Gradient>* m_data = nullptr;
	};

	void registerGradientData(asIScriptEngine* engine)
	{
		using WrapperType = DataWrapper<Gradient, GradientWrapper*>;
		int r = 0;
		r = engine->RegisterObjectType("Data<Gradient>", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod("Data<Gradient>", "Gradient@ getValue()",
			asMETHOD(WrapperType, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Data<Gradient>", "void setValue(Gradient@)",
			asMETHOD(WrapperType, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("Data<Gradient>", "int getCounter()",
			asMETHOD(WrapperType, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	void registerGradientType(asIScriptEngine* engine)
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

	void registerGradient(asIScriptEngine* engine)
	{
		registerGradientType(engine);
		registerGradientData(engine);
		int r = engine->RegisterObjectMethod("PandaObject", "Data<Gradient>@ createGradientData(bool, const string &in, const string &in)",
			asFUNCTION(createGradientData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
