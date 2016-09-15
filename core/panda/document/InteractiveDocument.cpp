#include <panda/document/InteractiveDocument.h>
#include <panda/document/DocumentSignals.h>

namespace panda {

InteractiveDocument::InteractiveDocument(gui::BaseGUI& gui)
	: RenderedDocument(gui)
	, m_mousePosition(initData("mouse position", "Current position of the mouse in the render view"))
	, m_mouseClick(initData(0, "mouse click", "1 if the left mouse button is pressed"))
{
	// Not connecting to the document, otherwise it would update the layers each time we get the time.
	m_mousePosition.setOutput(true);
	m_mousePosition.setReadOnly(true);

	m_mouseClick.setOutput(true);
	m_mouseClick.setReadOnly(true);
	m_mouseClick.setWidget("checkbox");
}

void InteractiveDocument::mouseMoveEvent(types::Point localPos, types::Point globalPos)
{ 
	m_mousePositionBuffer = localPos;
	m_signals->mouseMoveEvent.run(localPos, globalPos);
}

void InteractiveDocument::mouseButtonEvent(int button, bool isPressed, types::Point pos)
{
	if (button == 0)
	{
		if (m_mouseClickBuffer && !isPressed) // Pressed & released in 1 timestep, we will send 2 events
			m_mouseClickBuffer = -1;
		else
			m_mouseClickBuffer = isPressed;
	}

	m_signals->mouseButtonEvent.run(button, isPressed, pos);
}

void InteractiveDocument::keyEvent(int key, bool isPressed)
{
	m_signals->keyEvent.run(key, isPressed);
}

void InteractiveDocument::textEvent(const std::string& text)
{
	m_signals->textEvent.run(text);
}

void InteractiveDocument::updateDocumentData()
{
	m_mousePositionVal = m_mousePositionBuffer;
	if(m_mouseClickBuffer < 0)
	{
		m_mouseClickVal = 1;
		m_mouseClickBuffer = 0;
	}
	else
		m_mouseClickVal = m_mouseClickBuffer;

	RenderedDocument::updateDocumentData();

	m_mousePosition.setValue(m_mousePositionVal);
	m_mouseClick.setValue(m_mouseClickVal);
}

void InteractiveDocument::rewind()
{
	m_mousePositionVal = m_mousePositionBuffer;
	m_mousePosition.setValue(m_mousePositionBuffer);
	m_mouseClickVal = 0;
	m_mouseClick.setValue(0);

	RenderedDocument::rewind();
}

} // namespace panda


