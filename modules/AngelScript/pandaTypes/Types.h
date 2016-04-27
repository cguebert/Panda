#pragma once

class asIScriptEngine;

namespace panda
{

	void registerAllTypes(asIScriptEngine* engine);
	
	void registerColor(asIScriptEngine* engine);
	void registerInt(asIScriptEngine* engine);
	void registerIntVector(asIScriptEngine* engine);
	void registerFloat(asIScriptEngine* engine);
	void registerFloatVector(asIScriptEngine* engine);
	void registerGradient(asIScriptEngine* engine);
	void registerMesh(asIScriptEngine* engine);
	void registerPath(asIScriptEngine* engine);
	void registerPoint(asIScriptEngine* engine);
	void registerPolygon(asIScriptEngine* engine);
	void registerRect(asIScriptEngine* engine);
	void registerString(asIScriptEngine* engine);
	void registerAnimation(asIScriptEngine* engine);

} // namespace panda
