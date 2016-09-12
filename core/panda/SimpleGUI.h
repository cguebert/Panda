#ifndef BASEGUI_H
#define BASEGUI_H

#include <panda/core.h>
#include <panda/graphics/PointInt.h>

#include <functional>
#include <string>
#include <vector>

namespace panda
{

namespace gui
{

namespace buttons
{

enum 
{
	Ok = 0x00000400,
	Save = 0x00000800,
	SaveAll = 0x00001000,
	Open = 0x00002000,
	Yes = 0x00004000,
	YesToAll = 0x00008000,
	No = 0x00010000,
	NoToAll = 0x00020000,
	Abort = 0x00040000,
	Retry = 0x00080000,
	Ignore = 0x00100000,
	Close = 0x00200000,
	Cancel = 0x00400000,
	Discard = 0x00800000,
	Help = 0x01000000,
	Apply = 0x02000000,
	Reset = 0x04000000
};

} // namespace buttons

namespace menu
{
	enum
	{
		Object = 1 << 0,
		Data = 1 << 1,
		Link = 1 << 2,
		Image = 1 << 3,
		Tag = 1 << 4,
		Selection = 1 << 5 // Fill automatically based on the current selection
	/*	Annotation = 1 << 5,
		Group = 1 << 6,
		SelectedGreaterThan1 = 1 << 7,
		SelectedGreaterThan2 = 1 << 8 */
	};
}

enum class Color
{
	Window,
	Base,
	Text,
	Button,
	Light,
	Midlight,
	Dark,
	Highlight,
	HighlightedText
};

enum class MessageBoxType { about, critical, information, question, warning };

// A class used to communicate from the core (or the plugins) to the gui, if there is one
class PANDA_CORE_API BaseGUI
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

	virtual ~BaseGUI();

	// A combination of buttons can be set. Returns the button the user has clicked.
	virtual int messageBox(MessageBoxType type, const std::string& caption, const std::string& text, int buttons = buttons::Ok) = 0;

	virtual void contextMenu(panda::graphics::PointInt pos, int flags, const Actions& customActions = {}) = 0;

	virtual void updateView() = 0; // Update the OpenGL view

	virtual void contextMakeCurrent() = 0; // Make the OpenGL context corresponding to the view current
	virtual void contextDoneCurrent() = 0; // Release the context

	virtual void executeByUI(CallbackFunc func) = 0; // Put the function on a queue that will be executed on the UI thread

	virtual unsigned int getColor(Color color) = 0; // Get a gui color
};

} // namespace gui

} // namespace panda

#endif // BASEGUI_H
