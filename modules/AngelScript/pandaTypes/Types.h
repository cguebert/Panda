#pragma once

class asIScriptEngine;

namespace panda
{

	void registerAllTypes(asIScriptEngine* engine);
	
	void registerColor(asIScriptEngine* engine);
	void registerInt(asIScriptEngine* engine);
	void registerFloat(asIScriptEngine* engine);
	void registerGradient(asIScriptEngine* engine);
	void registerPoint(asIScriptEngine* engine);
	void registerRect(asIScriptEngine* engine);
	void registerString(asIScriptEngine* engine);

} // namespace panda
