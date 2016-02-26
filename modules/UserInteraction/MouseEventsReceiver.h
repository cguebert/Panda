#include <panda/types/Point.h>
#include <panda/messaging.h>

namespace panda {

class PandaDocument;
using types::Point;

class MouseEventsReceiver
{
protected:
	MouseEventsReceiver(PandaDocument* doc);

	virtual void mousePressed(panda::types::Point) {}
	virtual void mouseReleased(panda::types::Point) {}

private:
	msg::Observer m_observer;
	PandaDocument* m_document;

	void onMousePressed(panda::types::Point);
	void onMouseReleased(panda::types::Point);
};

} // namespace Panda

