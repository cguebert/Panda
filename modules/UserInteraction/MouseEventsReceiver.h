#include <QObject>
#include <panda/types/Point.h>

namespace panda {

class PandaDocument;
using types::Point;

class MouseEventsReceiver
{
public:
	MouseEventsReceiver(PandaDocument* doc);

	virtual void mousePressed(panda::types::Point) {}
	virtual void mouseReleased(panda::types::Point) {}
};

} // namespace Panda

