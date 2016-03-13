#include "ImGuiBase.h"
#include "ImGuiWrapper.h"

#include <panda/object/Dockable.h>
#include <panda/object/ObjectFactory.h>

#include "imgui/imgui.h"

namespace panda 
{

class ImGui_Window : public DockObject, public BaseImGuiContainer
{
public:
	PANDA_CLASS(ImGui_Window, DockObject)

	ImGui_Window(PandaDocument* doc)
		: DockObject(doc)
		, m_wrapper(ImGui_Wrapper::instance(doc))
	{
	}

	bool accepts(DockableObject* dockable) const override
	{
		return dynamic_cast<BaseImGuiObject*>(dockable) != nullptr;
	}

	void fillGui() override
	{
		for (auto object : getDockedObjects())
		{
			auto dockable = dynamic_cast<BaseImGuiObject*>(object);
			if (dockable)
				dockable->fillGui();
		}
	}

protected:
	std::shared_ptr<ImGui_Wrapper> m_wrapper;
};

int ImGui_WindowClass = RegisterObject<ImGui_Window>("ImGui/ImGui window").setDescription("Create an ImGui window");

//****************************************************************************//

class ImGui_TestWindow : public PandaObject, public BaseImGuiContainer
{
public:
	PANDA_CLASS(ImGui_TestWindow, PandaObject)

		ImGui_TestWindow(PandaDocument* doc)
		: PandaObject(doc)
		, m_wrapper(ImGui_Wrapper::instance(doc))
	{ }

	void fillGui() override
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow();
	}

protected:
	std::shared_ptr<ImGui_Wrapper> m_wrapper;
};

int ImGui_TestWindowClass = RegisterObject<ImGui_TestWindow>("ImGui/ImGui test window").setDescription("Show the ImGui test window");

} // namespace Panda
