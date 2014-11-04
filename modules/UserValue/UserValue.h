#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QObject>

namespace panda {

class TimedMethodObject : public QObject
{
	Q_OBJECT
public slots:
	virtual void onTimeout() {}
};

} // namespace Panda
