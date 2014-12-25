#include <QObject>
#include <panda/types/Point.h>

namespace panda {

class PandaDocument;
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

