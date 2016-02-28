#include "ScriptEngine.h"
#include "ObjectWrapper.h"

#include <assert.h>

#define AS_USE_STLNAMES 0
#include "addons/scriptstdstring/scriptstdstring.h"

namespace
{

void MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == asMSGTYPE_INFORMATION)
		type = "INFO";

	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
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
	int r = m_engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL); assert(r >= 0);

	ObjectWrapper::registerEngine(m_engine);
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
