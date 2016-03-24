#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Types.h"
#include "Gradient.h"

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>

#include <angelscript.h>
#include <cassert>

using panda::types::Animation;
using panda::types::Color;
using panda::types::Gradient;
using panda::types::Point;
using panda::BaseDataWrapper;
using panda::ObjectWrapper;

namespace panda
{
	template <class T>
	class AnimationDataWrapper;
}

namespace
{
	BaseDataWrapper* createFloatAnimationData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Animation<float>, panda::AnimationDataWrapper<float>>(input, name, help); }

	BaseDataWrapper* createPointAnimationData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Animation<Point>, panda::AnimationDataWrapper<Point>>(input, name, help); }

	BaseDataWrapper* createColorAnimationData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Animation<Color>, panda::AnimationDataWrapper<Color>>(input, name, help); }

	BaseDataWrapper* createGradientAnimationData(bool input, const std::string& name, const std::string& help, ObjectWrapper* wrapper)
	{ return wrapper->createData<Animation<Gradient>, panda::AnimationDataWrapper<Gradient>>(input, name, help); }

}

namespace panda 
{
	template <class T>
	class ValueTraits
	{
	public:
		using value_type = T;
		using value_vector = aatc::container::tempspec::vector<value_type>;

		static value_type fromValue(const T& val) { return val; }
		static T toValue(value_type val) { return val; }
		static value_vector* fromValues(const std::vector<T>& values, asIScriptEngine*)
		{ auto vec = new value_vector(); vec->container = values; return vec; }

		static std::string getReadTypeName(const std::string& typeName) { return typeName; }
		static std::string getWriteTypeName(const std::string& typeName) { return typeName; }
	};

	template <>
	class ValueTraits<Gradient>
	{
	public:
		using value_type = panda::GradientWrapper*;
		using value_vector = aatc::container::templated::vector;

		static value_type fromValue(const panda::types::Gradient& val) { return createGradientWrapper(val); }
		static panda::types::Gradient toValue(value_type val) { return getGradient(val); }
		static value_vector* fromValues(const std::vector<panda::types::Gradient>& values, asIScriptEngine* engine)
		{ return createGradientVectorWrapper(values, engine); }

		static std::string getReadTypeName(const std::string&) { return "Gradient@"; }
		static std::string getWriteTypeName(const std::string&) { return "const Gradient &in"; }
	};

	// Animation wrapper class
	template <class T>
	class AnimationWrapper
	{
	public:
		using traits = ValueTraits<T>;
		using value_type = typename traits::value_type;

		using AnimType = panda::types::Animation<T>;
		using position_vector = aatc::container::tempspec::vector<float>;
		using value_vector = typename traits::value_vector;

		static void factory(asIScriptGeneric* gen)
		{ *static_cast<AnimationWrapper**>(gen->GetAddressOfReturnLocation()) = new AnimationWrapper(gen->GetEngine()); }

		static AnimationWrapper* create(const AnimType& animation, asIScriptEngine* engine)
		{ return new AnimationWrapper(animation, engine); }

		AnimationWrapper(asIScriptEngine* engine) : m_engine(engine) {}
		AnimationWrapper(const AnimType& animation, asIScriptEngine* engine)
			: m_anim(animation), m_engine(engine) {}

		const AnimType& animation() const 
		{ return m_anim; }

		void addRef()
		{ m_refCount++; }

		void release()
		{ if (--m_refCount == 0) delete this; }
		
		bool operator==(const AnimationWrapper* wrapper) const
		{ return m_anim == wrapper->m_anim; }

		AnimationWrapper& assign(const AnimationWrapper* other)
		{ m_anim = other->m_anim; return *this; }

		int size() const
		{ return m_anim.size(); }
		void clear()
		{ m_anim.clear(); }

		void add(float position, value_type value)
		{ m_anim.add(position, traits::toValue(value)); }
		value_type get(float position) const
		{ return traits::fromValue(m_anim.get(position)); }

		value_type getAtIndex(int index) const
		{ return traits::fromValue(m_anim.valueAtIndex(index)); }
		void setAtIndex(int index, value_type val)
		{ m_anim.valueAtIndex(index) = traits::toValue(val); }

		void removeAtIndex(int index)
		{ m_anim.removeAtIndex(index); }

		void setInterpolation(int method)
		{ m_anim.setInterpolationInt(method); }
		int interpolation() const
		{ return static_cast<int>(m_anim.interpolation()); }

		void setExtend(int method)
		{ m_anim.setExtendInt(method); }
		int extend() const
		{ return static_cast<int>(m_anim.extend()); }

		position_vector* keys() const
		{
			auto vec = new position_vector();
			vec->container = m_anim.keys();
			return vec;
		}
		value_vector* values() const
		{ return traits::fromValues(m_anim.values(), m_engine); }

	private:
		int m_refCount = 1;
		AnimType m_anim;
		asIScriptEngine* m_engine = nullptr;
	};

	template <class T>
	class AnimationDataWrapper : public BaseDataWrapper
	{
	public:
		using AnimWrapper = AnimationWrapper<T>;
		using AnimData = Data<types::Animation<T>>;

		AnimationDataWrapper(AnimData* data, asIScriptEngine* engine)
			: m_data(data), m_engine(engine) { }

		AnimWrapper* getValue() const
		{ return AnimWrapper::create(m_data->getValue(), m_engine); }

		void setValue(const AnimWrapper* wrapper)
		{ m_data->setValue(wrapper->animation()); }

		int getCounter() const
		{ return m_data->getCounter(); }

	private:
		AnimData* m_data = nullptr;
		asIScriptEngine* m_engine = nullptr;
	};

	template <class T>
	void registerAnimationData(asIScriptEngine* engine, const std::string& typeName)
	{
		std::string capitalizedTypeName = typeName;
		capitalizedTypeName[0] = toupper(capitalizedTypeName[0]);
		const std::string animTypeName = capitalizedTypeName + "Animation";
		const std::string dataTypeName = animTypeName + "Data";
		auto dtn = dataTypeName.c_str();

		using AnimWrapper = AnimationDataWrapper<T>;

		int r = 0;
		r = engine->RegisterObjectType(dtn, 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectMethod(dtn, str(animTypeName + "@ getValue() const"),
			asMETHOD(AnimWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod(dtn, str("void setValue(const " + animTypeName + " &in)"),
			asMETHOD(AnimWrapper, setValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod(dtn, "int getCounter()",
			asMETHOD(AnimWrapper, getCounter), asCALL_THISCALL); assert(r >= 0);
	}

	template <class T>
	void registerAnimationType(asIScriptEngine* engine, const std::string& typeName)
	{
		std::string capitalizedTypeName = typeName;
		capitalizedTypeName[0] = toupper(capitalizedTypeName[0]);
		const std::string animTypeName = capitalizedTypeName + "Animation";
		auto atn = animTypeName.c_str();

		using traits = ValueTraits<T>;
		std::string readTypeName = traits::getReadTypeName(typeName);
		std::string writeTypeName = traits::getWriteTypeName(typeName);

		using AnimWrapper = AnimationWrapper<T>;

		int r = 0;

		r = engine->RegisterObjectType(atn, 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour(atn, asBEHAVE_FACTORY, str(animTypeName + "@ f()"), asFUNCTION(AnimWrapper::factory), asCALL_GENERIC); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour(atn, asBEHAVE_ADDREF, "void f()", asMETHOD(AnimWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour(atn, asBEHAVE_RELEASE, "void f()", asMETHOD(AnimWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, str(animTypeName + "& opAssign(const " + atn + "&in)"), asMETHOD(AnimWrapper, assign), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, str("bool opEquals(const " + animTypeName + " &in) const"), asMETHODPR(AnimWrapper, operator==, (const AnimWrapper*) const, bool), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, "int size() const", asMETHOD(AnimWrapper, size), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, "void clear()", asMETHOD(AnimWrapper, clear), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, str("void add(float, " + writeTypeName + ")"), asMETHOD(AnimWrapper, add), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, str(readTypeName + " get(float) const"), asMETHOD(AnimWrapper, get), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, str(readTypeName + " getAtIndex(int) const"), asMETHOD(AnimWrapper, getAtIndex), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, str("void setAtIndex(int, " + writeTypeName + ")"), asMETHOD(AnimWrapper, setAtIndex), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, "void removeAtIndex(int)", asMETHOD(AnimWrapper, removeAtIndex), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, "int interpolation() const", asMETHOD(AnimWrapper, interpolation), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, "void setInterpolation(int)", asMETHOD(AnimWrapper, setInterpolation), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, "int extend() const", asMETHOD(AnimWrapper, extend), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, "void setExtend(int)", asMETHOD(AnimWrapper, setExtend), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, str("vector<" + typeName + ">@ values() const"), asMETHOD(AnimWrapper, values), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, "vector<float>@ keys() const", asMETHOD(AnimWrapper, keys), asCALL_THISCALL); assert( r >= 0 );
	}

	template <class T>
	void registerAnimationT(asIScriptEngine* engine, const std::string& typeName)
	{
		registerAnimationType<T>(engine, typeName);
		registerAnimationData<T>(engine, typeName);
	}

	void registerAnimation(asIScriptEngine* engine)
	{
		registerAnimationT<float>(engine, "float");
		registerAnimationT<Point>(engine, "Point");
		registerAnimationT<Color>(engine, "Color");
		registerAnimationT<Gradient>(engine, "Gradient");

		int r = engine->RegisterObjectMethod("PandaObject", "FloatAnimationData@ createFloatAnimationData(bool, const string &in, const string &in)",
			asFUNCTION(createFloatAnimationData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "PointAnimationData@ createPointAnimationData(bool, const string &in, const string &in)",
			asFUNCTION(createPointAnimationData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "ColorAnimationData@ createColorAnimationData(bool, const string &in, const string &in)",
			asFUNCTION(createColorAnimationData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "GradientAnimationData@ createGradientAnimationData(bool, const string &in, const string &in)",
			asFUNCTION(createGradientAnimationData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda
