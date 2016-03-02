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
	std::string errorString(); // Return a copy of the error string and clears it

private:
	asIScriptEngine* m_engine = nullptr;
	std::string m_errorString;
};

inline asIScriptEngine* ScriptEngine::engine()
{ return m_engine; }

inline std::string ScriptEngine::errorString()
{ std::string tmp; tmp.swap(m_errorString); return tmp; }

} // namespace panda
