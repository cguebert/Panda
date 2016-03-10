#pragma once

#include <angelscript.h>

#include <memory>
#include <string>

namespace panda
{

class ScriptModuleHandle;

class ScriptEngine
{
public:
	static std::shared_ptr<ScriptEngine> instance();
	
	~ScriptEngine();

	std::shared_ptr<ScriptModuleHandle> newModule();

	asIScriptEngine* engine();

	void print(const std::string& str); // Can be used by the script (to show debug information, for example)
	std::string errorString(); // Return a copy of the error string and clears it

private:
	ScriptEngine();

	asIScriptEngine* m_engine = nullptr;
	std::string m_errorString;
	int m_nextModuleId = 1; // To ensure unique module names
};

class ScriptModuleHandle
{
public:
	ScriptModuleHandle(std::shared_ptr<ScriptEngine> engine, int moduleId);
	~ScriptModuleHandle();

	asIScriptFunction* getFunction(const std::string& signature);
	bool compileScript(const std::string& script);

private:
	std::shared_ptr<ScriptEngine> m_engine;
	asIScriptModule* m_module = nullptr;
	std::string m_moduleName;
};

inline asIScriptEngine* ScriptEngine::engine()
{ return m_engine; }

inline std::string ScriptEngine::errorString()
{ std::string tmp; tmp.swap(m_errorString); return tmp; }

} // namespace panda
