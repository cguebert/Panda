#pragma once

#include <panda/object/PandaObject.h>

class asIScriptContext;
class asIScriptFunction;

namespace panda 
{

class ObjectWrapper;
class ScriptEngine;
class ScriptModuleHandle;

class AS_ScriptedObject : public PandaObject
{
public:
	PANDA_CLASS(AS_ScriptedObject, PandaObject)

	AS_ScriptedObject(PandaDocument* parent);
	
	void setDirtyValue(const DataNode* caller) override;
	void update() override;

	void updateDatas();
	void setDebug(const std::string& str);

	void setLabel(const std::string& label);
	std::string getLabel() const override;

protected:
	Data<std::string> m_scriptText;
	Data<std::string> m_debugText;

	std::string m_label;

	std::shared_ptr<ScriptEngine> m_engine;
	std::shared_ptr<ScriptModuleHandle> m_module;
	std::shared_ptr<ObjectWrapper> m_wrapper;
	asIScriptContext* m_context = nullptr;
	asIScriptFunction *m_setupFunc = nullptr, *m_updateFunc = nullptr;

	std::vector<std::shared_ptr<BaseData>> m_createdDatas;
};

inline void AS_ScriptedObject::setLabel(const std::string& label)
{ m_label = label; }

inline std::string AS_ScriptedObject::getLabel() const
{ return m_label; }

} // namespace panda
