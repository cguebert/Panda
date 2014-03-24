#include <QObject>

namespace panda {

class TimedMethodObject : public QObject
{
	Q_OBJECT
public slots:
	virtual void onTimeout() {}
};

} // namespace Panda
