#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
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
		, m_centers(initData(&m_centers, "center", "Center of the disks"))
		, m_radiuses(initData(&m_radiuses, "radius", "Radius of each disk"))
		, m_selection(initData(&m_selection, "selected", "Selection status of each disk"))
		, m_centersPrevSize(-1)
	{
		addInput(&m_centers);
		addInput(&m_radiuses);
		addOutput(&m_selection);
	}

	void reset()
	{
		m_selection.getAccessor().clear();
		m_centersPrevSize = -1;
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
		const auto& centers = m_centers.getValue();
		const auto& radiuses = m_radiuses.getValue();
		auto selection = m_selection.getAccessor();
		int nbC = centers.size(), nbR = radiuses.size();
		if(m_centersPrevSize != nbC)
		{
			selection.resize(nbC);
			selection.wref().fill(0);
			m_centersPrevSize = nbC;
		}

		if(nbC && nbR)
		{
			for(auto& pt : m_clicsBuffer)
			{
				for(int i=0; i<nbC; ++i)
				{
					PReal radius = (nbR < nbC) ? radiuses[0] : radiuses[i];
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
	Data<QVector<Point>> m_centers;
	Data<QVector<PReal>> m_radiuses;
	Data<QVector<int>> m_selection;

	QVector<Point> m_clicsBuffer;
	int m_centersPrevSize;
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
		, m_rectangles(initData(&m_rectangles, "rectangle", "List of rectangles"))
		, m_selection(initData(&m_selection, "selected", "Selection status of each rectangle"))
		, m_rectanglesPrevSize(-1)
	{
		addInput(&m_rectangles);
		addOutput(&m_selection);
	}

	void reset()
	{
		m_selection.getAccessor().clear();
		m_rectanglesPrevSize = -1;
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
		const auto& rectangles = m_rectangles.getValue();
		auto selection = m_selection.getAccessor();
		int nbR = rectangles.size();
		if(m_rectanglesPrevSize != nbR)
		{
			selection.resize(nbR);
			selection.wref().fill(0);
			m_rectanglesPrevSize = nbR;
		}

		if(nbR)
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
	Data<QVector<Rect>> m_rectangles;
	Data<QVector<int>> m_selection;

	QVector<Point> m_clicsBuffer;
	int m_rectanglesPrevSize;
};

int UserInteraction_SelectionRectangleClass = RegisterObject<UserInteraction_SelectionRectangle>("Interaction/Rectangle selection").setDescription("Select rectangles with the mouse");

} // namespace Panda

