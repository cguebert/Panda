#include "ScriptEngine.h"
#include "ObjectWrapper.h"
#include "Types.h"

#include <assert.h>
#include <sstream>
#include <iostream>

#define AS_USE_STLNAMES 0
#include "addons/scriptstdstring/scriptstdstring.h"

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

ScriptEngine::ScriptEngine()
	: m_engine(asCreateScriptEngine())
{
	if (!m_engine)
		return;

	RegisterStdString(m_engine);
	//	RegisterStdStringUtils(engine);

	// Set the message callback to receive information on errors in human readable form.
	int r = m_engine->SetMessageCallback(asFUNCTION(MessageCallback), &m_errorString, asCALL_CDECL); assert(r >= 0);

	registerTypes(m_engine);
	registerData(m_engine);
	ObjectWrapper::registerObject(m_engine);
}

ScriptEngine::~ScriptEngine()
{
	if (m_engine)
		m_engine->ShutDownAndRelease();
}

bool ScriptEngine::compileScript(const std::string& script)
{
	if (!m_engine)
		return false;

	auto mod = m_engine->GetModule(0, asGM_ALWAYS_CREATE);
	int r = mod->AddScriptSection("script", script.c_str());
	if (r < 0) return false;

	r = mod->Build();
	if (r < 0) return false;
	return true;
}

asIScriptFunction* ScriptEngine::getFunction(const std::string& signature)
{
	if (!m_engine)
		return nullptr;
	return m_engine->GetModule(0)->GetFunctionByDecl(signature.c_str());
}

} // namespace panda
