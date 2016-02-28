#pragma once

#include <angelscript.h>

#include <string>

namespace panda
{

class ScriptEngine
{
public:
	ScriptEngine();
	~ScriptEngine();

	bool compileScript(const std::string& script);
	asIScriptFunction* getFunction(const std::string& signature);

	asIScriptEngine* engine();

private:
	asIScriptEngine* m_engine = nullptr;
};

inline asIScriptEngine* ScriptEngine::engine()
{ return m_engine; }

} // namespace panda
