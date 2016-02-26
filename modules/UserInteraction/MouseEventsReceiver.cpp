#include <panda/PandaDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/object/ObjectFactory.h>

#include "MouseEventsReceiver.h"

namespace panda
{

MouseEventsReceiver::MouseEventsReceiver(PandaDocument* doc)
	: m_document(doc)
{
	m_observer.get(doc->getSignals().mousePressed).connect<MouseEventsReceiver, &MouseEventsReceiver::onMousePressed>(this);
	m_observer.get(doc->getSignals().mouseReleased).connect<MouseEventsReceiver, &MouseEventsReceiver::onMouseReleased>(this);
}

void MouseEventsReceiver::onMousePressed(panda::types::Point pt)
{
	if (m_document->animationIsPlaying())
		mousePressed(pt);
}

void MouseEventsReceiver::onMouseReleased(panda::types::Point pt)
{
	if (m_document->animationIsPlaying())
		mouseReleased(pt);
}

}

panda::ModuleHandle userInteractionModule = REGISTER_MODULE
		.setDescription("Components used to interact with the mouse")
		.setLicense("GPL")
		.setVersion("1.0");
