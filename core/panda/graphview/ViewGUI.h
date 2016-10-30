#pragma once

#include <panda/types/Rect.h>

#include <panda/helper/Flags.h>

#include <functional>
#include <vector>

namespace panda
{

namespace graphview
{

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
	using MenuTypes = helper::Flags<MenuType>;

	class PANDA_CORE_API ViewGui
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

		~ViewGui();

		virtual void setCursor(Cursor cursor) = 0;
		virtual void restoreCursor() = 0;

		virtual void setStatusBarMessage(const std::string& msg) = 0;

		virtual void showToolTip(const types::Point& pos, const std::string& msg, const types::Rect& area = {}) = 0;

		virtual void contextMenu(const types::Point& pos, MenuTypes types, const Actions& customActions = {}) = 0;
	};

} // namespace graphview

} // namespace panda
