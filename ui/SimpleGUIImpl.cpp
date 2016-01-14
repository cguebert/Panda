#include <SimpleGUIImpl.h>
#include <MainWindow.h>
#include <QtWidgets>

SimpleGUIImpl::SimpleGUIImpl(MainWindow* mainWindow)
	: m_mainWindow(mainWindow)
{
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

void SimpleGUIImpl::updateView()
{
	if (m_mainWindow)
		m_mainWindow->updateOpenGLView();
}

void SimpleGUIImpl::executeByUI(panda::gui::CallbackFunc func)
{
	bool empty = true;
	{
		std::lock_guard<std::mutex> lock(m_functionsMutex);
		empty = m_functions.empty();
		m_functions.push_back(func);
	}

	if (empty) // Ask for the execution on the thread where the object was created
		QMetaObject::invokeMethod(this, "execute", Qt::QueuedConnection);
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
