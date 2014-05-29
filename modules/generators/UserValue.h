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

class BaseGeneratorUser : public PandaObject
{
public:
	PANDA_CLASS(BaseGeneratorUser, PandaObject)

	BaseGeneratorUser(PandaDocument *doc)
		: PandaObject(doc)
		, m_caption(initData(&m_caption, "caption", "The caption to use in the graph view"))
	{
	}

	const QString& getCaption()
	{
		return m_caption.getValue();
	}

protected:
	Data<QString> m_caption;
};

} // namespace Panda
