#include "ScriptEngine.h"
#include "pandaTypes/ObjectWrapper.h"
#include "pandaTypes/Types.h"

#include <assert.h>
#include <string>
#include <sstream>
#include <iostream>

#define AS_USE_STLNAMES 1
#include "addons/scriptstdstring/scriptstdstring.h"

#include "addons/scriptmath/scriptmath.h"
#include "addons/aatc/aatc.hpp"

namespace
{

void MessageCallback(const asSMessageInfo* msg, void* param)
{
	const char *type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == asMSGTYPE_INFORMATION)
		type = "INFO";

	
	std::stringstream ss;
	ss << msg->section << " (" << msg->row << ", " << msg->col << ") : " << type << " : " << msg->message;
	auto str = ss.str();

	if (msg->type == 0)
		std::cerr << str << std::endl;
	else
		std::cout << str << std::endl;

	auto& engineString = *static_cast<std::string*>(param);
	if (!engineString.empty())
		engineString += "\n";
	engineString += str;
}

}

namespace panda 
{

std::shared_ptr<ScriptEngine> ScriptEngine::instance()
{
	static std::weak_ptr<ScriptEngine> ptr;
	if (ptr.expired())
	{
		auto engine = std::shared_ptr<ScriptEngine>(new ScriptEngine());
		ptr = engine;
		return engine;
	}
	else
		return ptr.lock();
}

ScriptEngine::ScriptEngine()
	: m_engine(asCreateScriptEngine())
{
	if (!m_engine)
		return;

	// Set the message callback to receive information on errors in human readable form.
	int r = m_engine->SetMessageCallback(asFUNCTION(MessageCallback), &m_errorString, asCALL_CDECL); assert(r >= 0);

	RegisterStdString(m_engine);
	//	RegisterStdStringUtils(engine);

	// The scripts can directly print text
	r = m_engine->RegisterGlobalFunction("void print(const string &in)", asMETHOD(ScriptEngine, print), asCALL_THISCALL_ASGLOBAL, this); assert(r >= 0);

	RegisterScriptMath(m_engine);
	aatc::RegisterAllContainers(m_engine);
	
	r = m_engine->RegisterObjectType("PandaObject", 0, asOBJ_REF | asOBJ_NOCOUNT); assert(r >= 0);

	registerAllTypes(m_engine);
}

ScriptEngine::~ScriptEngine()
{
	if (m_engine)
		m_engine->ShutDownAndRelease();
}

std::shared_ptr<ScriptModuleHandle> ScriptEngine::newModule()
{
	return std::make_shared<ScriptModuleHandle>(instance(), m_nextModuleId++);
}

void ScriptEngine::print(const std::string& str)
{
	if (!m_errorString.empty())
		m_errorString += "\n";

	m_errorString += str;
}

//****************************************************************************//

ScriptModuleHandle::ScriptModuleHandle(std::shared_ptr<ScriptEngine> engine, int moduleId)
	: m_engine(engine)
{
	m_moduleName = "module" + std::to_string(moduleId);
}

ScriptModuleHandle::~ScriptModuleHandle()
{
	if (m_module)
		m_module->Discard();
}

bool ScriptModuleHandle::compileScript(const std::string& script)
{
	auto engine = m_engine->engine();
	if (!engine)
		return false;

	m_module = engine->GetModule(m_moduleName.c_str(), asGM_ALWAYS_CREATE);
	int r = m_module->AddScriptSection("script", script.c_str());
	if (r < 0) return false;

	r = m_module->Build();
	if (r < 0) return false;
	return true;
}

asIScriptFunction* ScriptModuleHandle::getFunction(const std::string& signature)
{
	if (!m_module)
		return nullptr;
	return m_module->GetFunctionByDecl(signature.c_str());
}

} // namespace panda
