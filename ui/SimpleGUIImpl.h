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
	void contextMenu(panda::graphics::PointInt pos, int flags, const Actions& customActions) override;
	void updateView() override;
	void contextMakeCurrent() override;
	void contextDoneCurrent() override;
	void executeByUI(CallbackFunc func) override;
	unsigned int getColor(panda::gui::Color color) override;

protected slots:
	void executeFunctions();

protected:
	MainWindow* m_mainWindow;

	std::vector<CallbackFunc> m_functions;
	std::mutex m_functionsMutex;
};
