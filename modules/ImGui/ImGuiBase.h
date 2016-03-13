#pragma once

namespace panda 
{

class BaseImGuiContainer // This is called by the ImGuiWrapper
{
public:
	virtual void fillGui() = 0; // Put ImGui commands here
};

class BaseImGuiObject // This is usually a dockable object connected to a dock container
{
public:
	virtual void fillGui() = 0; // Put ImGui commands here
};

} // namespace Panda
