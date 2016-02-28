#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include "ScriptEngine.h"
#include "ObjectWrapper.h"

#include <algorithm>

namespace panda
{
class AS_ScriptedObject;
}

namespace
{

static panda::AS_ScriptedObject* g_object = nullptr;
void print(const std::string& str);

static const std::string defaultScript =
R"~(void setup(PandaObject@ object)
{

}

void update()
{

}

)~";

}

namespace panda 
{

class AS_ScriptedObject : public PandaObject
{
public:
	PANDA_CLASS(AS_ScriptedObject, PandaObject)

	AS_ScriptedObject(PandaDocument* parent)
		: PandaObject(parent)
		, m_scriptText(initData(defaultScript, "script", "The script describing this object and its behavior"))
		, m_output(initData("output", "Debug string"))
		, m_wrapper(this)
	{
		m_context = m_engine.engine()->CreateContext();

		m_engine.engine()->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL);

		addInput(m_scriptText);
		addOutput(m_output);

		m_scriptText.setWidget("multiline");
	}

	void setDirtyValue(const DataNode* caller) override
	{
		if (caller == &m_scriptText)
		{
			if (m_engine.compileScript(m_scriptText.getValue()))
			{
				m_setupFunc = m_engine.getFunction("void setup(PandaObject@)");
				m_updateFunc = m_engine.getFunction("void update()");

				g_object = this;
				if (m_setupFunc)
				{
					m_context->Prepare(m_setupFunc);
					m_context->SetArgObject(0, &m_wrapper);
					
					if (m_context->Execute() == asEXECUTION_FINISHED)
						updateDatas();
				}
			}
		}
	}

	void update()
	{
		if (m_updateFunc)
		{
			m_context->Prepare(m_updateFunc);
			m_context->Execute();
		}
	}

	void updateDatas()
	{
		auto newDatas = m_wrapper.datas();
		m_wrapper.clear();

		auto oldDatas = m_createdDatas;
		m_createdDatas.clear();

		for (auto& newData : newDatas)
		{
			m_createdDatas.push_back(newData.data);

			if (newData.input)
				addInput(*newData.data);
			else
				addOutput(*newData.data);

			// Look for the same data, if it was already present
			auto newDataPtr = newData.data.get();
			auto it = std::find_if(oldDatas.begin(), oldDatas.end(), [&newDataPtr](const std::shared_ptr<BaseData>& data) {
				return data->getName() == newDataPtr->getName() 
					&& data->getDataTrait() == newDataPtr->getDataTrait();
			});

			// Then copy its value and set the parent
			if (it != oldDatas.end())
			{
				auto oldDataPtr = it->get();
				newDataPtr->copyValueFrom(oldDataPtr);

				auto parent = oldDataPtr->getParent();
				if (parent)
					dataSetParent(newDataPtr, parent);
			}
		}

		for (auto& oldData : oldDatas)
			removeData(oldData.get());

		if (!newDatas.empty() || !oldDatas.empty())
			emitModified();
	}

	void setDebug(const std::string& str)
	{
		m_output.setValue(str);
	}

protected:
	Data<std::string> m_scriptText;
	Data<std::string> m_output;

	ScriptEngine m_engine;
	ObjectWrapper m_wrapper;
	asIScriptContext* m_context = nullptr;
	asIScriptFunction *m_setupFunc = nullptr, *m_updateFunc = nullptr;

	std::vector<std::shared_ptr<BaseData>> m_createdDatas;
};

int AS_ScriptedObjectClass = RegisterObject<AS_ScriptedObject>("Angelscript").setDescription("Create a scriptable object");

//****************************************************************************//

} // namespace panda

namespace
{

void print(const std::string& str)
{
	if (g_object)
		g_object->setDebug(str);
}

}