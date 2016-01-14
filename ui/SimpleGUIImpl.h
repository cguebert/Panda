#pragma once

#include <panda/SimpleGUI.h>

#include <QObject>

#include <memory>
#include <mutex>
#include <vector>

class MainWindow;
class QWidget;

class SimpleGUIImpl : public QObject, public panda::gui::BaseGUI
{
	Q_OBJECT
public:
	SimpleGUIImpl(MainWindow* mainWindow);

	int messageBox(panda::gui::MessageBoxType type, const std::string& caption, const std::string& text, int buttons) override;
	void updateView() override;
	void executeByUI(panda::gui::CallbackFunc func) override;

protected:
	void executeFunctions();

	MainWindow* m_mainWindow;

	std::vector<panda::gui::CallbackFunc> m_functions;
	std::mutex m_functionsMutex;
};
