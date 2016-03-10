#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include "ScriptEngine.h"
#include "pandaTypes/ObjectWrapper.h"

#include <algorithm>

namespace
{

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
		, m_debugText(initData("debug", "Debug string"))
		, m_engine(ScriptEngine::instance())
		, m_wrapper(this, m_engine->engine())
	{
		m_module = m_engine->newModule();
		m_context = m_engine->engine()->CreateContext();

		addInput(m_scriptText);

		m_scriptText.setWidget("multiline");

		m_debugText.setWidget("multiline");
		m_debugText.setReadOnly(true);
	}

	void setDirtyValue(const DataNode* caller) override
	{
		if (caller == &m_scriptText)
		{
			m_updateFunc = nullptr;

			if (m_module->compileScript(m_scriptText.getValue()))
			{
				m_setupFunc = m_module->getFunction("void setup(PandaObject@)");

				if (m_setupFunc)
				{
					m_wrapper.clear();
					m_context->Prepare(m_setupFunc);
					m_context->SetArgObject(0, &m_wrapper);
					
					if (m_context->Execute() == asEXECUTION_FINISHED)
					{
						updateDatas();
						m_updateFunc = m_module->getFunction("void update()");
					}
				}
			}

			m_debugText.setValue(m_engine->errorString());
		}

		PandaObject::setDirtyValue(caller);
	}

	void update()
	{
		if (m_updateFunc)
		{
			m_context->Prepare(m_updateFunc);
			m_context->Execute();

			m_debugText.setValue(m_engine->errorString());
		}
	}

	void updateDatas()
	{
		auto newDatas = m_wrapper.datas();
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
					&& data->getDataTrait() == newDataPtr->getDataTrait()
					&& data->isInput() == newDataPtr->isInput()
					&& data->isOutput() == newDataPtr->isOutput();
			});

			// Then copy its value and set the parent
			if (it != oldDatas.end())
			{
				auto oldDataPtr = it->get();
				if (newDataPtr->isInput())
				{
					newDataPtr->copyValueFrom(oldDataPtr);

					auto parent = oldDataPtr->getParent();
					if (parent)
						dataSetParent(newDataPtr, parent);
				}
				else if (newDataPtr->isOutput())
				{
					auto outputs = oldDataPtr->getOutputs();
					for (auto output : outputs)
					{
						auto data = dynamic_cast<BaseData*>(output);
						if (data && data->getOwner())
							data->getOwner()->dataSetParent(data, newDataPtr);
					}
				}
			}
		}

		for (auto& oldData : oldDatas)
			removeData(oldData.get());

		if (!newDatas.empty() || !oldDatas.empty())
			emitModified();
	}

	void setDebug(const std::string& str)
	{
		m_debugText.setValue(str);
	}

protected:
	Data<std::string> m_scriptText;
	Data<std::string> m_debugText;

	std::shared_ptr<ScriptEngine> m_engine;
	std::shared_ptr<ScriptModuleHandle> m_module;
	ObjectWrapper m_wrapper;
	asIScriptContext* m_context = nullptr;
	asIScriptFunction *m_setupFunc = nullptr, *m_updateFunc = nullptr;

	std::vector<std::shared_ptr<BaseData>> m_createdDatas;
};

int AS_ScriptedObjectClass = RegisterObject<AS_ScriptedObject>("Angelscript").setDescription("Create a scriptable object");

} // namespace panda
