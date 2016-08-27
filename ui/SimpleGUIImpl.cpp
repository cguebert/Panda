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

int SimpleGUIImpl::messageBox(panda::gui::MessageBoxType type, const std::string& caption, const std::string& text, int buttons)
{
	using panda::gui::MessageBoxType;
	switch (type)
	{
	case MessageBoxType::about:
		QMessageBox::about(m_mainWindow, QString::fromStdString(caption), QString::fromStdString(text));
		return 0;
	case MessageBoxType::critical:
		return QMessageBox::critical(m_mainWindow, QString::fromStdString(caption), QString::fromStdString(text), static_cast<QMessageBox::StandardButtons>(buttons));
	case MessageBoxType::information:
		return QMessageBox::information(m_mainWindow, QString::fromStdString(caption), QString::fromStdString(text), static_cast<QMessageBox::StandardButtons>(buttons));
	case MessageBoxType::question:
		return QMessageBox::question(m_mainWindow, QString::fromStdString(caption), QString::fromStdString(text), static_cast<QMessageBox::StandardButtons>(buttons));
	case MessageBoxType::warning:
		return QMessageBox::warning(m_mainWindow, QString::fromStdString(caption), QString::fromStdString(text), static_cast<QMessageBox::StandardButtons>(buttons));
	}

	return 0;
}

void SimpleGUIImpl::contextMenu(panda::graphics::PointInt pos, int flags, const Actions& customActions)
{
	if (!flags && customActions.empty())
		return;

	QMenu menu(m_mainWindow);

	m_mainWindow->fillContextMenu(menu, flags);

	for (const auto& action : customActions)
	{
		const auto& label = action.first;
		if (label.empty() && !menu.isEmpty())
			menu.addSeparator();
		if (!label.empty())
			menu.addAction(QString::fromStdString(label), action.second);
	}
	
	if(!menu.actions().empty())
		menu.exec({ pos.x, pos.y });
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
