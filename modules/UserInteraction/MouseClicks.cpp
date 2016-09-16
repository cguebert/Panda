#include <panda/document/InteractiveDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Rect.h>

#include <chrono>

#include "MouseEventsReceiver.h"

namespace panda {

using types::Point;
using types::Rect;

class UserInteraction_MouseClicks : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_MouseClicks, PandaObject)

	UserInteraction_MouseClicks(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_accumulate(initData("accumulate", "If true, keep previous clics"))
		, m_clics(initData("position", "Position of the mouse clic"))
	{
		addInput(m_accumulate);
		addOutput(m_clics);

		m_accumulate.setWidget("checkbox");
	}

	void reset()
	{
		m_clics.getAccessor().clear();
		m_clicsBuffer.clear();
	}

	void beginStep()
	{
		PandaObject::beginStep();
		setDirtyValue(this);
	}

	void endStep()
	{
		PandaObject::endStep();
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
	Data<int> m_accumulate;
	Data<std::vector<Point>> m_clics;
	std::vector<Point> m_clicsBuffer;
};

int UserInteraction_MouseClicksClass = RegisterObject<UserInteraction_MouseClicks, InteractiveDocument>("Interaction/Mouse clicks").setDescription("Detect mouse clicks");

//****************************************************************************//

class UserInteraction_MouseDoubleClicks : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_MouseDoubleClicks, PandaObject)

	UserInteraction_MouseDoubleClicks(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_accumulate(initData("accumulate", "If true, keep previous double clics"))
		, m_doubleClickInterval(initData(0.4f, "db click interval", "Max duration beteen clicks in a double click"))
		, m_clics(initData("position", "Position of the mouse double clic"))
	{
		addInput(m_accumulate);
		addOutput(m_clics);

		m_accumulate.setWidget("checkbox");
	}

	void reset()
	{
		m_clics.getAccessor().clear();
		m_clicsBuffer.clear();
	}

	void beginStep()
	{
		PandaObject::beginStep();
		setDirtyValue(this);
	}

	void endStep()
	{
		PandaObject::endStep();
		updateIfDirty();
	}

	void mousePressed(panda::types::Point pos)
	{
		using namespace std::chrono;
		auto now = time_point_cast<milliseconds>(high_resolution_clock::now()).time_since_epoch().count();
		long long elapsed = 0;
		if (m_clicTime)
			elapsed = m_clicTime - now;

		bool doubleClick = elapsed < m_doubleClickInterval.getValue() * 1000	// Not too long ago
			&& (pos - m_currentClic).norm2() < 100; // Not too far (10 pixels)

		if (doubleClick)
		{
			m_clicsBuffer.push_back(m_currentClic);
			m_clicTime = 0;
		}
		else
		{
			m_clicTime = now;
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
	Data<std::vector<Point>> m_clics;
	Data<int> m_accumulate;
	Data<float> m_doubleClickInterval;

	Point m_currentClic;
	std::vector<Point> m_clicsBuffer;
	long long m_clicTime;
};

int UserInteraction_MouseDoubleClicksClass = RegisterObject<UserInteraction_MouseDoubleClicks, InteractiveDocument>("Interaction/Double clicks").setDescription("Detect mouse double clicks");

//****************************************************************************//

class UserInteraction_Button : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_Button, PandaObject)

	UserInteraction_Button(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_buttons(initData("button", "Area of the button"))
		, m_toggle(initData(false, "toggle", "If true, create a toggle button"))
		, m_status(initData("status", "Status of the button"))
	{
		addInput(m_buttons);
		addInput(m_toggle);
		addOutput(m_status);

		m_toggle.setWidget("checkbox");
	}

	void reset()
	{
		auto status = m_status.getAccessor();
		status.clear();
		status.resize(m_buttons.getValue().size());
		m_clicsBuffer.clear();
	}

	void beginStep()
	{
		PandaObject::beginStep();
		setDirtyValue(this);
	}

	void endStep()
	{
		PandaObject::endStep();
		updateIfDirty();
	}

	void mousePressed(panda::types::Point pos)
	{
		m_clicsBuffer.push_back(pos);
		setDirtyValue(this);
	}

	void update()
	{
		const auto& buttons = m_buttons.getValue();
		auto status = m_status.getAccessor();
		if(buttons.size() != status.size())
			status.resize(buttons.size());

		bool toggle = m_toggle.getValue() != 0;
		if(!toggle)
			status.wref().assign(status.size(), 0);

		for(int i=0, nb=buttons.size(); i<nb; ++i)
		{
			const auto& rect = buttons[i];
			for(const auto& pt : m_clicsBuffer)
			{
				if(rect.contains(pt))
				{
					if(toggle)
						status[i] = !status[i];
					else
						status[i] = true;
				}
			}
		}

		m_clicsBuffer.clear();
	}

protected:
	Data<std::vector<Rect>> m_buttons;
	Data<int> m_toggle;
	Data<std::vector<int>> m_status;
	std::vector<Point> m_clicsBuffer;
};

int UserInteraction_ButtonClass = RegisterObject<UserInteraction_Button, InteractiveDocument>("Interaction/Button").setDescription("Detect mouse clicks in a rectangle");

} // namespace Panda

