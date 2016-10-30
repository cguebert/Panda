#pragma once

#include <panda/SimpleGUI.h>

#include <mutex>
#include <vector>

class SimpleGUIImpl : public panda::gui::BaseGUI
{
public:
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
