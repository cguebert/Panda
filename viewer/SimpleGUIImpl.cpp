#include "SimpleGUIImpl.h"

#include <iostream>

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
