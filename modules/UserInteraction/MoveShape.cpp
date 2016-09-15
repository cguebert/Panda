#include <panda/object/PandaObject.h>
#include <panda/document/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Rect.h>

#include "MouseEventsReceiver.h"

namespace panda {

using types::Point;
using types::Rect;

class UserInteraction_MoveDisk : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_MoveDisk, PandaObject)

	UserInteraction_MoveDisk(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_input(initData("input", "Input positions"))
		, m_output(initData("output", "Output positions"))
		, m_radiuses(initData("radius", "Radius of each disk"))
		, m_counter(-1), m_currentSelection(-1)
	{
		addInput(m_input);
		addInput(m_radiuses);
		addOutput(m_output);

		m_radiuses.getAccessor().push_back(5.0);
	}

	void reset()
	{
		m_output.getAccessor().clear();
		m_clicsBuffer.clear();
		m_counter = -1;
		m_currentSelection = -1;
		setDirtyValue(this);
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
		m_clicsBuffer.push_back(std::make_pair(true, pos));
	}

	void mouseReleased(panda::types::Point pos)
	{
		m_clicsBuffer.push_back(std::make_pair(false, pos));
	}

	void update()
	{
		const auto& radiuses = m_radiuses.getValue();
		auto output = m_output.getAccessor();
		int nbPts = output.size(), nbR = radiuses.size();
		if(m_counter != m_input.getCounter())
		{
			output = m_input.getValue();
			m_counter = m_input.getCounter();
		}
		else if(nbPts && nbR)
		{
			for(auto& action : m_clicsBuffer)
			{
				if(action.first)
				{
					for(int i=0; i<nbPts; ++i)
					{
						float radius = (nbR < nbPts) ? radiuses[0] : radiuses[i];
						if((action.second - output[i]).norm2() < radius * radius)
						{
							m_currentSelection = i;
							m_moveStartPos = action.second;
							break;
						}
					}
				}
				else
				{
					if(m_currentSelection != -1)
					{
						output[m_currentSelection] += action.second - m_moveStartPos;
						m_currentSelection = -1;
					}
				}
			}

			if(m_currentSelection != -1)
			{
				Point currentPos = parentDocument()->getMousePosition();
				output[m_currentSelection] += currentPos - m_moveStartPos;
				m_moveStartPos = currentPos;
			}
		}

		m_clicsBuffer.clear();
	}

protected:
	Data<std::vector<Point>> m_input, m_output;
	Data<std::vector<float>> m_radiuses;

	std::vector<std::pair<bool, Point>> m_clicsBuffer;
	Point m_moveStartPos;
	int m_counter, m_currentSelection;
};

int UserInteraction_MoveDiskClass = RegisterObject<UserInteraction_MoveDisk>("Interaction/Move disk").setDescription("Move disks with the mouse");

//****************************************************************************//

class UserInteraction_MoveRectangle : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_MoveRectangle, PandaObject)

	UserInteraction_MoveRectangle(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_input(initData("input", "Input rectangles"))
		, m_output(initData("output", "Output rectangles"))
		, m_counter(-1), m_currentSelection(-1)
	{
		addInput(m_input);
		addOutput(m_output);
	}

	void reset()
	{
		m_output.getAccessor().clear();
		m_clicsBuffer.clear();
		m_counter = -1;
		m_currentSelection = -1;
		setDirtyValue(this);
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
		m_clicsBuffer.push_back(std::make_pair(true, pos));
	}

	void mouseReleased(panda::types::Point pos)
	{
		m_clicsBuffer.push_back(std::make_pair(false, pos));
	}

	void update()
	{
		auto output = m_output.getAccessor();
		int nbR = output.size();
		if(m_counter != m_input.getCounter())
		{
			output = m_input.getValue();
			m_counter = m_input.getCounter();
		}
		else if(nbR)
		{
			for(auto& action : m_clicsBuffer)
			{
				if(action.first)
				{
					for(int i=0; i<nbR; ++i)
					{
						if(output[i].contains(action.second))
						{
							m_currentSelection = i;
							m_moveStartPos = action.second;
							break;
						}
					}
				}
				else
				{
					if(m_currentSelection != -1)
					{
						output[m_currentSelection].translate(action.second - m_moveStartPos);
						m_currentSelection = -1;
					}
				}
			}

			if(m_currentSelection != -1)
			{
				Point currentPos = parentDocument()->getMousePosition();
				output[m_currentSelection].translate(currentPos - m_moveStartPos);
				m_moveStartPos = currentPos;
			}
		}

		m_clicsBuffer.clear();
	}

protected:
	Data<std::vector<Rect>> m_input, m_output;

	std::vector<std::pair<bool, Point>> m_clicsBuffer;
	Point m_moveStartPos;
	int m_counter, m_currentSelection;
};

int UserInteraction_MoveRectangleClass = RegisterObject<UserInteraction_MoveRectangle>("Interaction/Move rectangle").setDescription("Move rectangles with the mouse");

} // namespace Panda

