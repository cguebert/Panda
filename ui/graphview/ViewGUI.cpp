#include <ui/graphview/ViewGUI.h>
#include <ui/graphview/GraphView.h>
#include <ui/MainWindow.h>

#include <QApplication>
#include <QMenu>
#include <QToolTip>

namespace
{
	inline QPoint convert(const panda::types::Point& pt)
	{ return QPointF{ pt.x, pt.y }.toPoint(); }

	inline QRect convert(const panda::types::Rect& r)
	{ return QRectF{ r.left(), r.top(), r.right(), r.bottom() }.toRect(); }
}

namespace graphview
{

	ViewGui::ViewGui(GraphView& view, MainWindow* mainWindow)
		: m_view(view)
		, m_mainWindow(mainWindow)
	{
	}

	void ViewGui::setCursor(Cursor cursor)
	{
		QApplication::setOverrideCursor(static_cast<Qt::CursorShape>(cursor));
	}

	void ViewGui::restoreCursor()
	{
		QApplication::restoreOverrideCursor();
	}

	void ViewGui::setStatusBarMessage(const std::string& msg)
	{
		m_mainWindow->showStatusBarMessage(QString::fromStdString(msg));
	}

	void ViewGui::showToolTip(const panda::types::Point& pos, const std::string& msg, const panda::types::Rect& area)
	{
		const auto gPos = m_view.mapToGlobal(convert(pos));
		const auto gArea = QRect(m_view.mapToGlobal(convert(area.topLeft())), m_view.mapToGlobal(convert(area.bottomRight())));
		if (area.empty())
			QToolTip::showText(gPos, QString::fromStdString(msg), &m_view);
		else
			QToolTip::showText(gPos, QString::fromStdString(msg), &m_view, gArea);
	}

	void ViewGui::contextMenu(const panda::types::Point& pos, MenuTypes types, const Actions& customActions)
	{
		if (!types && customActions.empty())
			return;

		QMenu menu(m_mainWindow);

		m_mainWindow->fillContextMenu(menu, types);

		std::vector<QAction*> tempActions;
		for (const auto& action : customActions)
		{
			const auto& label = action.menuName;
			if (label.empty() && !menu.isEmpty())
				menu.addSeparator();
			if (!label.empty())
			{
				auto actionPtr = menu.addAction(QString::fromStdString(label), action.callback);
				tempActions.push_back(actionPtr);
				actionPtr->setParent(m_mainWindow); // So that the status tip can be shown on the status bar of the main window
				if (!action.statusTip.empty())
					actionPtr->setStatusTip(QString::fromStdString(action.statusTip));
			}
		}

		if (!menu.actions().empty())
			menu.exec(m_view.mapToGlobal(convert(pos)));

		for (auto action : tempActions)
			action->deleteLater();
	}

} // namespace graphview
