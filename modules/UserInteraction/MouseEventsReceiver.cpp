#include <panda/document/PandaDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/object/ObjectFactory.h>

#include "MouseEventsReceiver.h"

namespace panda
{

MouseEventsReceiver::MouseEventsReceiver(PandaDocument* doc)
	: m_document(doc)
{
	m_observer.get(doc->getSignals().mouseButtonEvent).connect<MouseEventsReceiver, &MouseEventsReceiver::onMouseButtonEvent>(this);
}

void MouseEventsReceiver::onMouseButtonEvent(int buttonId, bool isPressed, panda::types::Point position)
{
	if (m_document->animationIsPlaying() && buttonId == 0)
	{
		if (isPressed)
			mousePressed(position);
		else
			mouseReleased(position);
	}
}

}

panda::ModuleHandle userInteractionModule = REGISTER_MODULE
		.setDescription("Components used to interact with the mouse")
		.setLicense("GPL")
		.setVersion("1.0");
