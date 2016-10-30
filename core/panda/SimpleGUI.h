#ifndef BASEGUI_H
#define BASEGUI_H

#include <panda/core.h>

#include <functional>
#include <string>
#include <vector>

namespace panda
{

namespace gui
{

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

	virtual ~BaseGUI();

	virtual void updateView() = 0; // Update the OpenGL view

	virtual void contextMakeCurrent() = 0; // Make the OpenGL context corresponding to the view current
	virtual void contextDoneCurrent() = 0; // Release the context

	virtual void executeByUI(CallbackFunc func) = 0; // Put the function on a queue that will be executed on the UI thread

	virtual unsigned int getColor(Color color) = 0; // Get a gui color
};

} // namespace gui

} // namespace panda

#endif // BASEGUI_H
