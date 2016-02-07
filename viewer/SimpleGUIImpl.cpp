#include "SimpleGUIImpl.h"

#include <iostream>

int SimpleGUIImpl::messageBox(panda::gui::MessageBoxType type, const std::string& caption, const std::string& text, int buttons)
{
	using panda::gui::MessageBoxType;
	switch (type)
	{
	case MessageBoxType::about:
	case MessageBoxType::information:
	case MessageBoxType::question:
		std::cout << text << std::endl;
		return 0;
	case MessageBoxType::critical:
	case MessageBoxType::warning:
		std::cerr << text << std::endl;
		return 0;
	}

	return 0;
}

void SimpleGUIImpl::updateView()
{
}

void SimpleGUIImpl::contextMakeCurrent()
{
}

void SimpleGUIImpl::contextDoneCurrent()
{
}

void SimpleGUIImpl::executeByUI(panda::gui::CallbackFunc func)
{
	std::lock_guard<std::mutex> lock(m_functionsMutex);
	m_functions.push_back(func);
}

void SimpleGUIImpl::executeFunctions()
{
	std::vector<panda::gui::CallbackFunc> functions;
	{
		std::lock_guard<std::mutex> lock(m_functionsMutex);
		m_functions.swap(functions);
	}

	for (auto& func : functions)
		func();
}
