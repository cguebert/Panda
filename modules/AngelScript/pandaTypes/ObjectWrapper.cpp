#include "ObjectWrapper.h"

namespace panda
{

	void registerObject(asIScriptEngine* engine)
	{
		int r = engine->RegisterObjectType("PandaObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		r = engine->RegisterObjectType("BaseData", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);
		
		r = engine->RegisterObjectMethod("PandaObject", "void addInput(BaseData &in data)", asMETHOD(ObjectWrapper, setInput), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectMethod("PandaObject", "void addOutput(BaseData &in data)", asMETHOD(ObjectWrapper, setOutput), asCALL_THISCALL); assert( r >= 0 );
	}

	void ObjectWrapper::setInput(const BaseDataWrapper* wrapper)
	{
		auto it = std::find_if(m_datas.begin(), m_datas.end(), [wrapper](const DataInfo& di) {
			return di.dataWrapper.get() == wrapper;
		});
		if (it != m_datas.end())
			it->input = true;
	}

	void ObjectWrapper::setOutput(const BaseDataWrapper* wrapper)
	{
		auto it = std::find_if(m_datas.begin(), m_datas.end(), [wrapper](const DataInfo& di) {
			return di.dataWrapper.get() == wrapper;
		});
		if (it != m_datas.end())
			it->output = true;
	}

} // namespace panda
