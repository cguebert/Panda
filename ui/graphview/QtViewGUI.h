#pragma once

#include <panda/graphview/ViewGUI.h>

class MainWindow;

namespace graphview
{

	class QtViewWrapper;

	class QtViewGui : public ViewGui
	{
	public:
		QtViewGui(QtViewWrapper& viewWrapper, MainWindow* mainWindow);

		void setCursor(Cursor cursor) override;
		void restoreCursor() override;

		void setStatusBarMessage(const std::string& msg) override;

		void showToolTip(const panda::types::Point& pos, const std::string& msg, const panda::types::Rect& area = {}) override;

		void contextMenu(const panda::types::Point& pos, MenuTypes types, const Actions& customActions = {}) override;

	protected:
		QtViewWrapper& m_viewWrapper;
		MainWindow* m_mainWindow;
	};

}