#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include "MouseEventsReceiver.h"

namespace panda
{

MouseEventsReceiver::MouseEventsReceiver(PandaDocument* doc)
{
	m_observer.get(doc->m_mousePressedSignal).connect<MouseEventsReceiver, &MouseEventsReceiver::mousePressed>(this);
	m_observer.get(doc->m_mouseReleasedSignal).connect<MouseEventsReceiver, &MouseEventsReceiver::mouseReleased>(this);
}

}

panda::ModuleHandle userInteractionModule = REGISTER_MODULE
		.setDescription("Components used to interact with the mouse")
		.setLicense("GPL")
		.setVersion("1.0");
