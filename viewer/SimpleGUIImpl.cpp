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

void SimpleGUIImpl::contextMenu(panda::graphics::PointInt pos, int flags, const Actions& customActions)
{
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

void SimpleGUIImpl::executeByUI(CallbackFunc func)
{
	std::lock_guard<std::mutex> lock(m_functionsMutex);
	m_functions.push_back(func);
}

void SimpleGUIImpl::executeFunctions()
{
	std::vector<CallbackFunc> functions;
	{
		std::lock_guard<std::mutex> lock(m_functionsMutex);
		m_functions.swap(functions);
	}

	for (auto& func : functions)
		func();
}

unsigned int SimpleGUIImpl::getColor(panda::gui::Color color)
{
	return 0;
}
