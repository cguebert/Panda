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
		, m_title(initData(std::string("Panda"), "title", "Title of the ImGui window"))
		, m_position(initData(types::Point(100, 100), "position", "Position of the ImGui window"))
		, m_size(initData(types::Point(0, 0), "size", "Size of the ImGui window. Set to (0,0) for auto fit"))
	{
		addInput(m_title);
	}

	bool accepts(DockableObject* dockable) const override
	{
		return dynamic_cast<BaseImGuiObject*>(dockable) != nullptr;
	}

	void fillGui() override
	{
		types::Point pos = m_position.getValue(), size = m_size.getValue();
		ImGui::SetNextWindowPos({ pos.x, pos.y });
		ImGui::SetNextWindowSize({ size.x, size.y });

		ImGui::Begin(m_title.getValue().c_str());
		
		// Save the window position
		ImVec2 nPos = ImGui::GetWindowPos();
		if (pos.x != nPos.x || pos.y != nPos.y)
			m_position.setValue({ nPos.x, nPos.y });

		// If the window is collapsed, we can stop here
		if (ImGui::IsWindowCollapsed())
		{
			ImGui::End();
			return;
		}

		// Draw the widgets
		for (auto object : getDockedObjects())
		{
			auto dockable = dynamic_cast<BaseImGuiObject*>(object);
			if (dockable)
				dockable->fillGui();
		}

		// Save the window size
		ImVec2 nSize = ImGui::GetWindowSize();
		if (size != types::Point::zero() && (size.x != nSize.x || size.y != nSize.y))
			m_size.setValue({ nSize.x, nSize.y });

		ImGui::End();
	}

protected:
	std::shared_ptr<ImGui_Wrapper> m_wrapper;

	Data<std::string> m_title;
	Data<types::Point> m_position, m_size;
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
