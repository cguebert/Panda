#pragma once

#include <panda/types/Rect.h>

namespace graphview
{

	class GraphView;

	enum class Cursor
	{
		Arrow,
		UpArrow,
		Cross,
		Wait,
		IBeam,
		SizeVer,
		SizeHor,
		SizeBDiag,
		SizeFDiag,
		SizeAll,
		Blank,
		SplitV,
		SplitH,
		PointingHand,
		Forbidden,
		WhatsThis,
		Busy,
		OpenHand,
		ClosedHand,
		DragCopy,
		DragMove,
		DragLink
	};

	class ViewGUI
	{
	public:
		ViewGUI(GraphView& view);

		void setCursor(Cursor cursor);
		void restoreCursor();

		void setStatusBarMessage(const std::string& msg);

		void showToolTip(const panda::types::Point& pos, const std::string& msg, const panda::types::Rect& area = {});

	protected:
		GraphView& m_view;
	};

}
