#include <panda/types/Point.h>
#include <panda/messaging.h>

namespace panda {

class PandaDocument;
using types::Point;

class MouseEventsReceiver
{
public:
	MouseEventsReceiver(PandaDocument* doc);

	virtual void mousePressed(panda::types::Point) {}
	virtual void mouseReleased(panda::types::Point) {}

protected:
	msg::Observer m_observer;
};

} // namespace Panda

