#pragma once

#include <panda/types/Rect.h>

#include <panda/helper/Flags.h>

#include <functional>

class MainWindow;

namespace graphview
{

	class QtViewWrapper;

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

	enum class MenuType
	{
		Object = 1 << 0,
		Data = 1 << 1,
		Link = 1 << 2,
		Image = 1 << 3,
		Tag = 1 << 4,
		Selection = 1 << 5 // Fill automatically based on the current selection
	};

	// Can use EventModifier as a flag
	using MenuTypes = panda::helper::Flags<MenuType>;

	class ViewGui
	{
	public:
		using CallbackFunc = std::function<void()>;
		struct Action
		{
			Action(const std::string& name, const std::string& tip, CallbackFunc func)
				: menuName(name), statusTip(tip), callback(func) {}

			std::string menuName, statusTip;
			CallbackFunc callback;
		};
		using Actions = std::vector<Action>;

		ViewGui(QtViewWrapper& viewWrapper, MainWindow* mainWindow);

		void setCursor(Cursor cursor);
		void restoreCursor();

		void setStatusBarMessage(const std::string& msg);

		void showToolTip(const panda::types::Point& pos, const std::string& msg, const panda::types::Rect& area = {});

		void contextMenu(const panda::types::Point& pos, MenuTypes types, const Actions& customActions = {});

	protected:
		QtViewWrapper& m_viewWrapper;
		MainWindow* m_mainWindow;
	};

}
