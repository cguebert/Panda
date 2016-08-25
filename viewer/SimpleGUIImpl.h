#pragma once

#include <panda/SimpleGUI.h>

#include <mutex>
#include <vector>

class SimpleGUIImpl : public panda::gui::BaseGUI
{
public:
	int messageBox(panda::gui::MessageBoxType type, const std::string& caption, const std::string& text, int buttons) override;
	void contextMenu(panda::graphics::PointInt pos, int flags, const Actions& customActions) override;
	void updateView() override;
	void contextMakeCurrent() override;
	void contextDoneCurrent() override;
	void executeByUI(CallbackFunc func) override;
	unsigned int getColor(panda::gui::Color color) override;

	void executeFunctions();

protected:
	std::vector<CallbackFunc> m_functions;
	std::mutex m_functionsMutex;
};
