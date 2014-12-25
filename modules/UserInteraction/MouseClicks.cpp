#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <QApplication>
#include <QElapsedTimer>

#include "MouseEventsReceiver.h"

namespace panda {

using types::Point;

class UserInteraction_MouseClicks : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_MouseClicks, PandaObject)

	UserInteraction_MouseClicks(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_accumulate(initData(&m_accumulate, "accumulate", "If true, keep previous clics"))
		, m_clics(initData(&m_clics, "position", "Position of the mouse clic"))
	{
		addInput(&m_accumulate);
		addOutput(&m_clics);

		m_accumulate.setWidget("checkbox");
	}

	void reset()
	{
		m_clics.getAccessor().clear();
		m_clicsBuffer.clear();
	}

	void beginStep()
	{
		setDirtyValue(this);
	}

	void endStep()
	{
		updateIfDirty();
	}

	void mousePressed(panda::types::Point pos)
	{
		m_clicsBuffer.push_back(pos);
	}

	void update()
	{
		auto acc = m_clics.getAccessor();

		if(!m_accumulate.getValue())
			acc.clear();

		for(const auto& pt : m_clicsBuffer)
			acc.push_back(pt);

		m_clicsBuffer.clear();
	}

protected:
	Data<QVector<Point>> m_clics;
	Data<int> m_accumulate;
	QVector<Point> m_clicsBuffer;
};

int UserInteraction_MouseClicksClass = RegisterObject<UserInteraction_MouseClicks>("Interaction/Mouse clicks").setDescription("Detect mouse clicks");

//****************************************************************************//

class UserInteraction_MouseDoubleClicks : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_MouseDoubleClicks, PandaObject)

	UserInteraction_MouseDoubleClicks(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_accumulate(initData(&m_accumulate, "accumulate", "If true, keep previous double clics"))
		, m_clics(initData(&m_clics, "position", "Position of the mouse double clic"))
	{
		addInput(&m_accumulate);
		addOutput(&m_clics);

		m_accumulate.setWidget("checkbox");

		QApplication* app = dynamic_cast<QApplication*>(QCoreApplication::instance());
		if(app)
			m_doubleClickInterval = app->doubleClickInterval();
		else
			m_doubleClickInterval = 400;

		m_clicTimer.invalidate();
	}

	void reset()
	{
		m_clics.getAccessor().clear();
		m_clicsBuffer.clear();
	}

	void beginStep()
	{
		setDirtyValue(this);
	}

	void endStep()
	{
		updateIfDirty();
	}

	void mousePressed(panda::types::Point pos)
	{
		if(m_clicTimer.isValid() // Had a first clic
				&& m_clicTimer.elapsed() < m_doubleClickInterval // Not too long ago
				&& (pos-m_currentClic).norm2() < 100) // Not too far (10 pixels)
		{
			m_clicsBuffer.push_back(m_currentClic);
			m_clicTimer.invalidate();
		}
		else
		{
			m_clicTimer.start();
			m_currentClic = pos;
		}
	}

	void update()
	{
		auto acc = m_clics.getAccessor();

		if(!m_accumulate.getValue())
			acc.clear();

		for(const auto& pt : m_clicsBuffer)
			acc.push_back(pt);

		m_clicsBuffer.clear();
	}

protected:
	Data<QVector<Point>> m_clics;
	Data<int> m_accumulate;

	int m_doubleClickInterval;
	Point m_currentClic;
	QVector<Point> m_clicsBuffer;
	QElapsedTimer m_clicTimer;
};

int UserInteraction_MouseDoubleClicksClass = RegisterObject<UserInteraction_MouseDoubleClicks>("Interaction/Double clicks").setDescription("Detect mouse double clicks");

} // namespace Panda

