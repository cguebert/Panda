#include <panda/ObjectFactory.h>

#include "MouseEventsReceiver.h"

namespace panda
{

MouseEventsReceiver::MouseEventsReceiver(PandaDocument* doc)
{
	connect(doc, SIGNAL(mousePressed(panda::types::Point)), this, SLOT(mousePressed(panda::types::Point)));
	connect(doc, SIGNAL(mouseReleased(panda::types::Point)), this, SLOT(mouseReleased(panda::types::Point)));
}

}

panda::ModuleHandle userInteractionModule = REGISTER_MODULE
		.setDescription("Components used to interact with the mouse")
		.setLicense("GPL")
		.setVersion("1.0");
