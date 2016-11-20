#include <ui/graphview/QtViewGUI.h>
#include <ui/graphview/QtViewWrapper.h>
#include <ui/MainWindow.h>

#include <QApplication>
#include <QInputDialog>
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

	QtViewGui::QtViewGui(QtViewWrapper& viewWrapper, MainWindow* mainWindow)
		: m_viewWrapper(viewWrapper)
		, m_mainWindow(mainWindow)
	{
	}

	void QtViewGui::setCursor(panda::graphview::Cursor cursor)
	{
		QApplication::setOverrideCursor(static_cast<Qt::CursorShape>(cursor));
	}

	void QtViewGui::restoreCursor()
	{
		QApplication::restoreOverrideCursor();
	}

	void QtViewGui::setStatusBarMessage(const std::string& msg)
	{
		m_mainWindow->showStatusBarMessage(QString::fromStdString(msg));
	}

	void QtViewGui::showToolTip(const panda::types::Point& pos, const std::string& msg, const panda::types::Rect& area)
	{
		const auto gPos = m_viewWrapper.mapToGlobal(convert(pos));
		const auto gArea = QRect(m_viewWrapper.mapToGlobal(convert(area.topLeft())), m_viewWrapper.mapToGlobal(convert(area.bottomRight())));
		if (area.empty())
			QToolTip::showText(gPos, QString::fromStdString(msg), &m_viewWrapper);
		else
			QToolTip::showText(gPos, QString::fromStdString(msg), &m_viewWrapper, gArea);
	}

	void QtViewGui::contextMenu(const panda::types::Point& pos, panda::graphview::MenuTypes types, const Actions& customActions)
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
			menu.exec(m_viewWrapper.mapToGlobal(convert(pos)));

		for (auto action : tempActions)
			action->deleteLater();
	}

	std::string QtViewGui::getText(const std::string& label, const std::string& input)
	{
		return QInputDialog::getText(&m_viewWrapper, QString::fromStdString(label), "Value: ", QLineEdit::Normal, QString::fromStdString(input)).toStdString();
	}

} // namespace graphview
