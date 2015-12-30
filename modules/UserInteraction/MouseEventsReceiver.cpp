#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include "MouseEventsReceiver.h"

namespace panda
{

MouseEventsReceiver::MouseEventsReceiver(PandaDocument* doc)
{
	m_observer.get(doc->m_mousePressedSignal).connect([this](panda::types::Point pt) { mousePressed(pt); });
	m_observer.get(doc->m_mouseReleasedSignal).connect([this](panda::types::Point pt) { mouseReleased(pt); });
}

}

panda::ModuleHandle userInteractionModule = REGISTER_MODULE
		.setDescription("Components used to interact with the mouse")
		.setLicense("GPL")
		.setVersion("1.0");
