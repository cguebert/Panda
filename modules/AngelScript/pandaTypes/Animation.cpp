#include "DataWrapper.h"
#include "ObjectWrapper.h"
#include "Types.h"

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>

#include <angelscript.h>
#include <cassert>

using panda::types::Animation;
using panda::types::Color;
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
}

namespace panda 
{

	// Animaion wrapper class
	template <class T>
	class AnimationWrapper
	{
	public:
		using value_type = T;
		using AnimType = panda::types::Animation<value_type>;
		using position_vector = aatc::container::tempspec::vector<float>;
		using value_vector = aatc::container::tempspec::vector<value_type>;

		static AnimationWrapper* factory()
		{ return new AnimationWrapper(); }

		static AnimationWrapper* create(const AnimType& animation)
		{ return new AnimationWrapper(animation); }

		AnimationWrapper() {}
		AnimationWrapper(const AnimType& animation)
			: m_anim(animation) {}

		const AnimType& animation() const 
		{ return m_anim; }

		void addRef()
		{ m_refCount++; }

		void release()
		{ if (--m_refCount == 0) delete this; }
		
		bool operator==(const AnimationWrapper* wrapper) const
		{ return m_anim == wrapper->m_anim; }

		AnimationWrapper* assign(AnimationWrapper* other)
		{ m_anim = other->m_anim; addRef(); other->release(); return this; }

		int size() const
		{ return m_anim.size(); }
		void clear()
		{ m_anim.clear(); }

		void add(float position, value_type value)
		{ m_anim.add(position, value); }
		value_type get(float position) const
		{ return m_anim.get(position); }

		value_type getAtIndex(int index) const
		{ return m_anim.valueAtIndex(index); }
		void setAtIndex(int index, value_type val)
		{ m_anim.valueAtIndex(index) = val; }

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
		{
			auto vec = new value_vector();
			vec->container = m_anim.values();
			return vec;
		}

	private:
		int m_refCount = 1;
		AnimType m_anim;
	};

	template <class T>
	class AnimationDataWrapper : public BaseDataWrapper
	{
	public:
		using AnimWrapper = AnimationWrapper<T>;
		using AnimData = Data<types::Animation<T>>;

		AnimationDataWrapper(AnimData* data, asIScriptEngine*)
			: m_data(data) { }

		AnimWrapper* getValue() const
		{ return AnimWrapper::create(m_data->getValue()); }

		void setValue(AnimWrapper* wrapper)
		{ m_data->setValue(wrapper->animation()); wrapper->release(); }

		int getCounter() const
		{ return m_data->getCounter(); }

	private:
		AnimData* m_data = nullptr;
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
		r = engine->RegisterObjectMethod(dtn, str(animTypeName + "@ getValue()"),
			asMETHOD(AnimWrapper, getValue), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod(dtn, str("void setValue(" + animTypeName + "@)"),
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

		using AnimWrapper = AnimationWrapper<T>;

		int r = 0;

		r = engine->RegisterObjectType(atn, 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour(atn, asBEHAVE_FACTORY, str(animTypeName + "@ f()"), asFUNCTION(AnimWrapper::factory), asCALL_CDECL); assert( r >= 0 );

		r = engine->RegisterObjectBehaviour(atn, asBEHAVE_ADDREF, "void f()", asMETHOD(AnimWrapper, addRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour(atn, asBEHAVE_RELEASE, "void f()", asMETHOD(AnimWrapper, release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, str(animTypeName + "@ opAssign(" + atn + "@)"), asMETHOD(AnimWrapper, assign), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, str("bool opEquals(const " + animTypeName + "@) const"), asMETHODPR(AnimWrapper, operator==, (const AnimWrapper*) const, bool), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, "int size() const", asMETHOD(AnimWrapper, size), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, "void clear()", asMETHOD(AnimWrapper, clear), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, str("void add(float, " + typeName + ")"), asMETHOD(AnimWrapper, add), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, str(typeName + " get(float) const"), asMETHOD(AnimWrapper, get), asCALL_THISCALL); assert( r >= 0 );

		r = engine->RegisterObjectMethod(atn, str(typeName + " getAtIndex(int) const"), asMETHOD(AnimWrapper, getAtIndex), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod(atn, str("void setAtIndex(int, " + typeName + ")"), asMETHOD(AnimWrapper, setAtIndex), asCALL_THISCALL); assert( r >= 0 );

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

		int r = engine->RegisterObjectMethod("PandaObject", "FloatAnimationData@ createFloatAnimationData(bool, const string &in, const string &in)",
			asFUNCTION(createFloatAnimationData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "PointAnimationData@ createPointAnimationData(bool, const string &in, const string &in)",
			asFUNCTION(createPointAnimationData), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("PandaObject", "ColorAnimationData@ createColorAnimationData(bool, const string &in, const string &in)",
			asFUNCTION(createColorAnimationData), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}

} // namespace panda