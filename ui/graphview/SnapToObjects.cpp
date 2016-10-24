#include <ui/graphview/SnapToObjects.h>

#include <ui/graphview/GraphView.h>
#include <ui/graphview/Viewport.h>
#include <ui/graphview/ObjectRenderersList.h>
#include <ui/graphview/object/ObjectRenderer.h>

#include <panda/object/PandaObject.h>

using Point = panda::types::Point;
using Rect = panda::types::Rect;

namespace graphview
{

	SnapToObjects::SnapToObjects(GraphView& view)
		: m_view(view)
	{
	}

	void SnapToObjects::prepareSnapTargets(object::ObjectRenderer* objRnd)
	{
		m_snapTargetsY.clear();

		float y = objRnd->getPosition().y;
		// For y, try to make the data links horizontal
		// First for inputs
		for (auto input : objRnd->getObject()->getInputDatas())
		{
			Rect dataRect;
			if (objRnd->getDataRect(input, dataRect))
			{
				auto dataHeight = dataRect.center().y - y;
				for (auto input2 : input->getInputs())
				{
					auto data2 = dynamic_cast<panda::BaseData*>(input2);
					if (data2 && data2->getOwner())
					{
						auto owner = data2->getOwner();
						auto objRnd = m_view.objectRenderers().get(owner);
						if (objRnd)
						{
							if (objRnd->getDataRect(data2, dataRect))
								m_snapTargetsY.insert(dataRect.center().y - dataHeight);
						}
					}
				}
			}
		}

		// Then for outputs
		for (auto output : objRnd->getObject()->getOutputDatas())
		{
			Rect dataRect;
			if (objRnd->getDataRect(output, dataRect))
			{
				auto dataHeight = dataRect.center().y - y;
				for (auto output2 : output->getOutputs())
				{
					auto data2 = dynamic_cast<panda::BaseData*>(output2);
					if (data2 && data2->getOwner())
					{
						auto owner = data2->getOwner();
						auto objRnd = m_view.objectRenderers().get(owner);
						if (objRnd)
						{
							if (objRnd->getDataRect(data2, dataRect))
								m_snapTargetsY.insert(dataRect.center().y - dataHeight);
						}
					}
				}
			}
		}
	}

	void SnapToObjects::computeSnapDelta(object::ObjectRenderer* objRnd, const Point& position)
	{
		m_snapDelta = Point();
		const float snapMaxDist = 5;

		auto comparator = [](float pos) {
			return [pos](const float& lhs, const float& rhs) {
				return fabs(pos - lhs) < fabs(pos - rhs);
			};
		};

		// We look for the closest object above and the closest below
		const float filterRatio = 0.66f, filterDist = 50;
		auto selectedHeight = objRnd->getObjectSize().y;
		auto displayRect = m_view.viewport().displayRect();
		auto m1 = std::numeric_limits<float>::lowest(), m2 = std::numeric_limits<float>::max();
		Point abovePos(m1, m1), belowPos(m2, m2);
		float aboveDist { m2 }, belowDist { m2 };
		bool hasInsideObject = false;
		std::set<float> snapTargetsX;
		for (const auto objRnd : m_view.objectRenderers().getOrdered())
		{
			if (objRnd == objRnd || !objRnd->acceptsMagneticSnap())
				continue;

			auto area = objRnd->getVisualArea();
			if (displayRect.intersects(area)) // Only if visible in the current viewport
			{
				auto pos = objRnd->getPosition();
				if (pos.y + area.height() < position.y)
				{
					// Distance from the bottom left corner of this one and the top left of the selected
					auto dist = Point(pos.x - position.x, pos.y + area.height() - position.y).norm();
					if (dist < aboveDist)
					{
						aboveDist = dist;
						abovePos = Point(pos.x, pos.y + area.height());
					}
				}
				else if (pos.y > position.y + selectedHeight)
				{
					// Distance from the top left corner of this one and the bottom left of the selected
					auto dist = Point(pos.x - position.x, pos.y - (position.y + selectedHeight)).norm();
					if (dist < belowDist)
					{
						belowDist = dist;
						belowPos = Point(pos.x, pos.y - selectedHeight);
					}
				}
				else if (qAbs(pos.x - position.x) < filterDist) // The selected one intersects the y axis of this one, and is close enough on the x axis
				{
					snapTargetsX.insert(pos.x);
					hasInsideObject = true;
				}
			}
		}

		if (hasInsideObject)
		{
			// Only take the other ones if their are close
			if (qAbs(abovePos.y - position.y) < filterDist)
				snapTargetsX.insert(abovePos.x);
			if (qAbs(belowPos.y - position.y) < filterDist)
				snapTargetsX.insert(belowPos.x);
		}
		else
		{
			// We only take the closest if the other one is at least 50% further
			if (aboveDist < belowDist * filterRatio && belowDist > filterDist)
				snapTargetsX.insert(abovePos.x);
			else if (belowDist < aboveDist * filterRatio && aboveDist > filterDist)
				snapTargetsX.insert(belowPos.x);
			else
			{
				snapTargetsX.insert(abovePos.x);
				snapTargetsX.insert(belowPos.x);
			}
		}

		auto minIter = std::min_element(snapTargetsX.begin(), snapTargetsX.end(), comparator(position.x));
		if (minIter != snapTargetsX.end())
		{
			float x = *minIter;
			if (qAbs(x - position.x) < snapMaxDist)
				m_snapDelta.x = x - position.x;
		}

		minIter = std::min_element(m_snapTargetsY.begin(), m_snapTargetsY.end(), comparator(position.y));
		if (minIter != m_snapTargetsY.end())
		{
			float y = *minIter;
			if (qAbs(y - position.y) < snapMaxDist)
				m_snapDelta.y = y - position.y;
		}
	}

} // namespace graphview
