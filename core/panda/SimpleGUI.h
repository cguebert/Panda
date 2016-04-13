#ifndef BASEGUI_H
#define BASEGUI_H

#include <panda/core.h>

#include <functional>
#include <string>

namespace panda
{

namespace gui
{

namespace buttons
{

enum {
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

using CallbackFunc = std::function<void()>;

// A class used to communicate from the core (or the plugins) to the gui, if there is one
class PANDA_CORE_API BaseGUI
{
public:
	virtual ~BaseGUI();

	// A combination of buttons can be set. Returns the button the user has clicked.
	virtual int messageBox(MessageBoxType type, const std::string& caption, const std::string& text, int buttons = buttons::Ok) = 0;

	virtual void updateView() = 0; // Update the OpenGL view

	virtual void contextMakeCurrent() = 0; // Make the OpenGL context corresponding to the view current
	virtual void contextDoneCurrent() = 0; // Release the context

	virtual void executeByUI(CallbackFunc func) = 0; // Put the function on a queue that will be executed on the UI thread

	virtual unsigned int getColor(Color color) = 0; // Get a gui color
};

} // namespace gui

} // namespace panda

#endif // BASEGUI_H
