#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include "MouseEventsReceiver.h"

namespace panda
{

MouseEventsReceiver::MouseEventsReceiver(PandaDocument* doc)
{
	doc->connect(doc, &PandaDocument::mousePressed, [this](panda::types::Point pt) { mousePressed(pt); });
	doc->connect(doc, &PandaDocument::mouseReleased, [this](panda::types::Point pt) { mouseReleased(pt); });
}

}

panda::ModuleHandle userInteractionModule = REGISTER_MODULE
		.setDescription("Components used to interact with the mouse")
		.setLicense("GPL")
		.setVersion("1.0");
