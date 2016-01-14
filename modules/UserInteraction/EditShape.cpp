#include <panda/object/PandaObject.h>
#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Rect.h>

#include "MouseEventsReceiver.h"

namespace panda {

using types::Point;
using types::Rect;

class UserInteraction_EditRectangle : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_EditRectangle, PandaObject)

	UserInteraction_EditRectangle(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_input(initData("input", "Input rectangles"))
		, m_output(initData("output", "Output rectangles"))
		, m_handleRadius(initData((PReal)5, "handle radius", "Radius of the handles"))
		, m_centerHandles(initData("center", "Center handles"))
		, m_cornerHandles(initData("corner", "Corner handles"))
		, m_counter(-1), m_currentSelection(-1)
	{
		addInput(m_input);
		addInput(m_handleRadius);
		addOutput(m_output);
		addOutput(m_centerHandles);
		addOutput(m_cornerHandles);
	}

	void reset()
	{
		m_output.getAccessor().clear();
		m_centerHandles.getAccessor().clear();
		m_cornerHandles.getAccessor().clear();
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

	void editRectangle(Rect& rect, bool handle, Point delta, Point& center, Point& corner)
	{
		if(handle) // Center
		{
			rect.translate(delta);
			center += delta;
			corner += delta;
		}
		else // Corner
		{
			corner += delta;
			Point size = corner - center;
			size.x = fabs(size.x);
			size.y = fabs(size.y);
			rect.set(center.x - size.x, center.y - size.y,
					 center.x + size.x, center.y + size.y);
		}
	}

	void update()
	{
		auto output = m_output.getAccessor();
		auto centerH = m_centerHandles.getAccessor();
		auto cornerH = m_cornerHandles.getAccessor();
		PReal radius = m_handleRadius.getValue();
		PReal radius2 = radius*radius;
		int nbR = output.size();
		if(m_counter != m_input.getCounter())
		{
			output = m_input.getValue();
			nbR = output.size();
			centerH.resize(nbR);
			cornerH.resize(nbR);
			for(int i=0; i<nbR; ++i)
			{
				centerH[i] = output[i].center();
				cornerH[i] = output[i].bottomRight();
			}
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
						if((centerH[i]-action.second).norm2() < radius2)
						{
							m_currentSelection = i;
							m_selectedHandle = true;
							m_moveStartPos = action.second;
							break;
						}
						if((cornerH[i]-action.second).norm2() < radius2)
						{
							m_currentSelection = i;
							m_selectedHandle = false;
							m_moveStartPos = action.second;
							break;
						}
					}
				}
				else
				{
					if(m_currentSelection != -1)
					{
						editRectangle(output[m_currentSelection], m_selectedHandle,
									  action.second - m_moveStartPos,
									  centerH[m_currentSelection], cornerH[m_currentSelection]);
						centerH[m_currentSelection] = output[m_currentSelection].center();
						cornerH[m_currentSelection] = output[m_currentSelection].bottomRight();
						m_currentSelection = -1;
					}
				}
			}

			if(m_currentSelection != -1)
			{
				Point currentPos = m_parentDocument->getMousePosition();
				editRectangle(output[m_currentSelection], m_selectedHandle,
							  currentPos - m_moveStartPos,
							  centerH[m_currentSelection], cornerH[m_currentSelection]);
				m_moveStartPos = currentPos;
			}
		}

		m_clicsBuffer.clear();
	}

protected:
	Data<std::vector<Rect>> m_input, m_output;
	Data<PReal> m_handleRadius;
	Data<std::vector<Point>> m_centerHandles, m_cornerHandles;

	std::vector<std::pair<bool, Point>> m_clicsBuffer;
	Point m_moveStartPos;
	int m_counter, m_currentSelection;
	bool m_selectedHandle; // If true: center is selected; false: corner is selected
};

int UserInteraction_EditRectangleClass = RegisterObject<UserInteraction_EditRectangle>("Interaction/Edit rectangle").setDescription("Modify rectangles with the mouse");

//****************************************************************************//

} // namespace Panda

