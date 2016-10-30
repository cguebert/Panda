#include <ui/SimpleGUIImpl.h>
#include <ui/OpenGLRenderView.h>
#include <ui/MainWindow.h>
#include <QtWidgets>
#include <cassert>

SimpleGUIImpl::SimpleGUIImpl(MainWindow* mainWindow)
	: m_mainWindow(mainWindow)
{
	assert(mainWindow != nullptr);
}

void SimpleGUIImpl::updateView()
{
	m_mainWindow->getOpenGLView()->update();
}

void SimpleGUIImpl::contextMakeCurrent()
{
	m_mainWindow->getOpenGLView()->makeCurrent();
}

void SimpleGUIImpl::contextDoneCurrent()
{
	m_mainWindow->getOpenGLView()->doneCurrent();
}

void SimpleGUIImpl::executeByUI(CallbackFunc func)
{
	bool empty = true;
	{
		std::lock_guard<std::mutex> lock(m_functionsMutex);
		empty = m_functions.empty();
		m_functions.push_back(func);
	}

	if (empty) // Ask for the execution on the thread where the object was created
		QMetaObject::invokeMethod(this, "executeFunctions", Qt::QueuedConnection);
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
	using panda::gui::Color;
	auto pal = m_mainWindow->palette();
	switch (color)
	{
	case Color::Window: return pal.window().color().rgb();
	case Color::Base: return pal.base().color().rgb();
	case Color::Text: return pal.text().color().rgb();
	case Color::Button: return pal.button().color().rgb();
	case Color::Light: return pal.light().color().rgb();
	case Color::Midlight: return pal.midlight().color().rgb();
	case Color::Dark: return pal.dark().color().rgb();
	case Color::Highlight: return pal.highlight().color().rgb();
	case Color::HighlightedText: return pal.highlightedText().color().rgb();
	default:
		return 0;
	}
}
