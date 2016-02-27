#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <assert.h>
#include <angelscript.h>

#define AS_USE_STLNAMES 0
#include "addons/scriptstdstring/scriptstdstring.h"

namespace panda
{
class AS_ScriptedObject;
}

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

static panda::AS_ScriptedObject* g_object = nullptr;
void print(const std::string& str);

void configureEngine(asIScriptEngine* engine)
{
	RegisterStdString(engine);
//	RegisterStdStringUtils(engine);

	// Set the message callback to receive information on errors in human readable form.
	int r = engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL); assert(r >= 0);

	r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL); assert(r >= 0);
}

bool compileScript(asIScriptEngine* engine, const std::string& script)
{
	auto mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	int r = mod->AddScriptSection("script", script.c_str());
	if (r < 0) return false;

	r = mod->Build();
	if (r < 0) return false;
	return true;
}

static const std::string defaultScript =
R"~(void setup()
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
		, m_output(initData("output", "Debug string"))
		, m_engine(asCreateScriptEngine())
	{
		addInput(m_scriptText);
		addOutput(m_output);

		m_scriptText.setWidget("multiline");

		if (m_engine)
			configureEngine(m_engine);
	}

	~AS_ScriptedObject()
	{
		m_engine->ShutDownAndRelease();
	}

	void prepareContext()
	{
		if (!m_context)
			m_context = m_engine->CreateContext();

		m_setupFunc = m_engine->GetModule(0)->GetFunctionByDecl("void setup()");
	}

	void setDirtyValue(const DataNode* caller) override
	{
		if (!m_engine)
			return;

		if (caller == &m_scriptText)
		{
			if (compileScript(m_engine, m_scriptText.getValue()))
			{
				prepareContext();

				g_object = this;
				if (m_setupFunc)
					m_context->Prepare(m_setupFunc);
				m_context->Execute();
			}
		}
	}

	void update()
	{
	
	}

	void setDebug(const std::string& str)
	{
		m_output.setValue(str);
	}

protected:
	Data<std::string> m_scriptText;
	Data<std::string> m_output;

	asIScriptEngine* m_engine = nullptr;
	asIScriptContext* m_context = nullptr;
	asIScriptFunction* m_setupFunc = nullptr;
};

int AS_ScriptedObjectClass = RegisterObject<AS_ScriptedObject>("Angelscript").setDescription("Create a scriptable object");

//****************************************************************************//

} // namespace panda

namespace
{

void print(const std::string& str)
{
	if (g_object)
		g_object->setDebug(str);
}

}