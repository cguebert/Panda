#include <panda/PandaDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/object/ObjectFactory.h>

#include "MouseEventsReceiver.h"

namespace panda
{

MouseEventsReceiver::MouseEventsReceiver(PandaDocument* doc)
{
	m_observer.get(doc->getSignals().mousePressed).connect<MouseEventsReceiver, &MouseEventsReceiver::mousePressed>(this);
	m_observer.get(doc->getSignals().mouseReleased).connect<MouseEventsReceiver, &MouseEventsReceiver::mouseReleased>(this);
}

}

panda::ModuleHandle userInteractionModule = REGISTER_MODULE
		.setDescription("Components used to interact with the mouse")
		.setLicense("GPL")
		.setVersion("1.0");
