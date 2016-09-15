#pragma once

#include <panda/document/RenderedDocument.h>

namespace panda {

namespace gui
{
	class BaseGUI;
}

class PANDA_CORE_API InteractiveDocument : public RenderedDocument
{
public:
	PANDA_CLASS(InteractiveDocument, PandaObject)

	explicit InteractiveDocument(gui::BaseGUI& gui);

	types::Point getMousePosition() const;
	void mouseMoveEvent(types::Point localPos, types::Point globalPos);

	int getMouseClick() const; // Only gives the status of the left button
	void mouseButtonEvent(int button, bool isPressed, types::Point pos);

	void keyEvent(int key, bool isPressed);
	void textEvent(const std::string& text); // Unicode inputs

	void rewind() override;

protected:
	void updateDocumentData() override;

	types::Point m_mousePositionVal;
	int m_mouseClickVal = 0;

	Data<types::Point> m_mousePosition;
	Data<int> m_mouseClick;

	types::Point m_mousePositionBuffer;
	int m_mouseClickBuffer = 0;
};

//****************************************************************************//

inline types::Point InteractiveDocument::getMousePosition() const
{ return m_mousePositionVal; }

inline int InteractiveDocument::getMouseClick() const
{ return m_mouseClickVal; }

} // namespace panda

