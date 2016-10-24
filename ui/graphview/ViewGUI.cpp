#include <ui/graphview/ViewGUI.h>
#include <ui/graphview/GraphView.h>

#include <QApplication>
#include <QToolTip>

namespace
{
	inline QPoint convert(const panda::types::Point& pt)
	{ return QPointF{ pt.x, pt.y }.toPoint(); }

	inline QRect convert(const panda::types::Rect& r)
	{ return QRectF { r.left(), r.top(), r.right(), r.bottom() }.toRect(); }
}

namespace graphview
{

	ViewGUI::ViewGUI(GraphView& view)
		: m_view(view)
	{
	}

	void ViewGUI::setCursor(Cursor cursor)
	{
		QApplication::setOverrideCursor(static_cast<Qt::CursorShape>(cursor));
	}

	void ViewGUI::restoreCursor()
	{
		QApplication::restoreOverrideCursor();
	}

	void ViewGUI::setStatusBarMessage(const std::string& msg)
	{
		emit m_view.showStatusBarMessage(QString::fromStdString(msg));
	}

	void ViewGUI::showToolTip(const panda::types::Point& pos, const std::string& msg, const panda::types::Rect& area)
	{
		if (area.empty())
			QToolTip::showText(convert(pos), QString::fromStdString(msg), &m_view);
		else
			QToolTip::showText(convert(pos), QString::fromStdString(msg), &m_view, convert(area));
	}

	
} // namespace graphview
