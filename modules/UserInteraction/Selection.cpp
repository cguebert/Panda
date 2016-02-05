#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/types/Rect.h>

#include "MouseEventsReceiver.h"

namespace panda {

using types::Point;
using types::Rect;

class UserInteraction_SelectionDisk : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_SelectionDisk, PandaObject)

	UserInteraction_SelectionDisk(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_centers(initData("center", "Center of the disks"))
		, m_radiuses(initData("radius", "Radius of each disk"))
		, m_selection(initData("selected", "Selection status of each disk"))
		, m_counter(-1)
	{
		addInput(m_centers);
		addInput(m_radiuses);
		addOutput(m_selection);
	}

	void reset()
	{
		m_selection.getAccessor().clear();
		m_counter = -1;
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
		m_clicsBuffer.push_back(pos);
	}

	void update()
	{
		const auto& centers = m_centers.getValue();
		const auto& radiuses = m_radiuses.getValue();
		auto selection = m_selection.getAccessor();
		int nbC = centers.size(), nbR = radiuses.size();
		if(m_counter != m_centers.getCounter())
		{
			selection.wref().resize(nbC, 0);
			m_counter = m_centers.getCounter();
		}
		else if(nbC && nbR)
		{
			for(auto& pt : m_clicsBuffer)
			{
				for(int i=0; i<nbC; ++i)
				{
					float radius = (nbR < nbC) ? radiuses[0] : radiuses[i];
					if( (pt - centers[i]).norm2() < radius * radius)
					{
						selection[i] = !selection[i];
						break;
					}
				}
			}
		}

		m_clicsBuffer.clear();
	}

protected:
	Data<std::vector<Point>> m_centers;
	Data<std::vector<float>> m_radiuses;
	Data<std::vector<int>> m_selection;

	std::vector<Point> m_clicsBuffer;
	int m_counter;
};

int UserInteraction_SelectionDiskClass = RegisterObject<UserInteraction_SelectionDisk>("Interaction/Disk selection").setDescription("Select disks with the mouse");

//****************************************************************************//

class UserInteraction_SelectionRectangle : public PandaObject, public MouseEventsReceiver
{
public:
	PANDA_CLASS(UserInteraction_SelectionRectangle, PandaObject)

	UserInteraction_SelectionRectangle(PandaDocument *doc)
		: PandaObject(doc)
		, MouseEventsReceiver(doc)
		, m_rectangles(initData("rectangle", "List of rectangles"))
		, m_selection(initData("selected", "Selection status of each rectangle"))
		, m_counter(-1)
	{
		addInput(m_rectangles);
		addOutput(m_selection);
	}

	void reset()
	{
		m_selection.getAccessor().clear();
		m_counter = -1;
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
		m_clicsBuffer.push_back(pos);
	}

	void update()
	{
		const auto& rectangles = m_rectangles.getValue();
		auto selection = m_selection.getAccessor();
		int nbR = rectangles.size();
		if(m_counter != m_rectangles.getCounter())
		{
			selection.wref().resize(nbR, 0);
			m_counter = m_rectangles.getCounter();
		}
		else if(nbR)
		{
			for(auto& pt : m_clicsBuffer)
			{
				for(int i=0; i<nbR; ++i)
				{
					if(rectangles[i].contains(pt))
					{
						selection[i] = !selection[i];
						break;
					}
				}
			}
		}

		m_clicsBuffer.clear();
	}

protected:
	Data<std::vector<Rect>> m_rectangles;
	Data<std::vector<int>> m_selection;

	std::vector<Point> m_clicsBuffer;
	int m_counter;
};

int UserInteraction_SelectionRectangleClass = RegisterObject<UserInteraction_SelectionRectangle>("Interaction/Rectangle selection").setDescription("Select rectangles with the mouse");

} // namespace Panda

