#pragma once

#include <panda/SimpleGUI.h>

#include <mutex>
#include <vector>

class SimpleGUIImpl : public panda::gui::BaseGUI
{
public:
	int messageBox(panda::gui::MessageBoxType type, const std::string& caption, const std::string& text, int buttons) override;
	void updateView() override;
	void contextMakeCurrent() override;
	void contextDoneCurrent() override;
	void executeByUI(panda::gui::CallbackFunc func) override;
	unsigned int getColor(panda::gui::Color color) override;

	void executeFunctions();

protected:
	std::vector<panda::gui::CallbackFunc> m_functions;
	std::mutex m_functionsMutex;
};
