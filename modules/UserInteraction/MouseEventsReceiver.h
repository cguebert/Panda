#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>

namespace panda {

using types::Point;

class MouseEventsReceiver : public QObject
{
	Q_OBJECT

public:
	MouseEventsReceiver(PandaDocument* doc);

public slots:
	virtual void mousePressed(panda::types::Point) {}
	virtual void mouseReleased(panda::types::Point) {}
};

} // namespace Panda

