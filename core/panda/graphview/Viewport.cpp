#include <panda/graphview/Viewport.h>
#include <panda/graphview/GraphView.h>
#include <panda/graphview/ObjectsSelection.h>
#include <panda/graphview/ObjectRenderersList.h>
#include <panda/graphview/object/ObjectRenderer.h>

#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>

namespace panda
{

using types::Point;
using types::Rect;

namespace graphview
{

	Viewport::Viewport(GraphView& view)
		: m_view(view)
	{
	}

	Viewport::~Viewport() = default;

	void Viewport::zoomIn()
	{
		if(m_zoomLevel > 0)
		{
			const auto center = m_viewSize / 2;
			auto oldPos = center / m_zoomFactor;
			m_zoomLevel = std::max(m_zoomLevel - 10, 0);
			m_zoomFactor = (100 - m_zoomLevel) / 100.f;
			moveView(center / m_zoomFactor - oldPos);
		}
	}

	void Viewport::zoomOut()
	{
		if(m_zoomLevel < 90)
		{
			const auto center = m_viewSize / 2;
			auto oldPos = center / m_zoomFactor;
			m_zoomLevel = std::min(m_zoomLevel + 10, 90);
			m_zoomFactor = (100 - m_zoomLevel) / 100.f;
			moveView(center / m_zoomFactor - oldPos);
		}
	}

	void Viewport::zoomReset()
	{
		if(m_zoomLevel != 1)
		{
			const auto center = m_viewSize / 2;
			auto oldPos = center / m_zoomFactor;
			m_zoomLevel = 1;
			m_zoomFactor = 1.f;
			moveView(center / m_zoomFactor - oldPos);
		}
	}

	void Viewport::centerView()
	{
		const auto center = m_viewSize / 2;
		if(!m_objectsRect.empty())
			moveView(center / m_zoomFactor - m_objectsRect.center() + m_viewDelta);
	}

	void Viewport::showAll()
	{
		if(!m_objectsRect.empty())
		{
			float factorW = m_viewSize.x / (m_objectsRect.width() + 40);
			float factorH = m_viewSize.y / (m_objectsRect.height() + 40);
			m_zoomFactor = helper::bound(0.1f, std::min(factorW, factorH), 1.0f);
			m_zoomLevel = static_cast<int>(100 * (1.f - m_zoomFactor));
			moveView(m_viewSize / 2 / m_zoomFactor - m_objectsRect.center() + m_viewDelta);
		}
	}

	void Viewport::showAllSelected()
	{
		if(!m_view.selection().get().empty())
		{
			Rect selectedArea;
			for (const auto& obj : m_view.selection().get())
			{
				auto objRnd = m_view.objectRenderers().get(obj);
				if(objRnd)
					selectedArea |= objRnd->getVisualArea();
			}

			float factorW = m_viewSize.x / (selectedArea.width() + 40);
			float factorH = m_viewSize.y / (selectedArea.height() + 40);
			m_zoomFactor = helper::bound(0.1f, std::min(factorW, factorH), 1.0f);
			m_zoomLevel = static_cast<int>(100 * (1.f - m_zoomFactor));
			moveView(m_viewSize / 2 / m_zoomFactor - selectedArea.center() + m_viewDelta);
		}
	}

	void Viewport::moveSelectedToCenter()
	{
		if(!m_view.selection().get().empty())
		{
			const auto center = m_viewSize / 2;
			Point delta = center / m_zoomFactor - m_objectsRect.center() + m_viewDelta;

			for(const auto objRnd : m_view.selectedObjectsRenderers())
				objRnd->move(delta);

			updateObjectsRect();
		}
	}

	void Viewport::moveView(const types::Point& delta)
	{
		m_viewDelta -= delta;
		updateViewRect();
	}

	void Viewport::updateObjectsRect()
	{
		if (m_view.isLoading())
			return;

		m_objectsRect = Rect();
		for(const auto& objRnd : m_view.objectRenderers().getOrdered())
			m_objectsRect |= objRnd->getVisualArea();

		updateViewRect();
	}

	void Viewport::updateViewRect()
	{
		m_viewRect = Rect::fromSize(m_objectsRect.topLeft() * m_zoomFactor, m_objectsRect.size() * m_zoomFactor);
		if(!m_view.objectRenderers().getOrdered().empty())
			m_viewRect.adjust(-5, -5, 5, 5);

		modified.run();
	}

	void Viewport::setViewport(const types::Rect& globalArea)
	{
		Rect zoomRect = Rect(globalArea.topLeft() / m_zoomFactor, globalArea.bottomRight() / m_zoomFactor)
			.translated(m_viewDelta)
			.canonicalized();

		if (zoomRect.area() > 1000)
		{
			float factorW = m_viewSize.x / (zoomRect.width() + 40);
			float factorH = m_viewSize.y / (zoomRect.height() + 40);
			m_zoomFactor = helper::bound(0.1f, std::min(factorW, factorH), 1.0f);
			m_zoomLevel = static_cast<int>(100 * (1.f - m_zoomFactor));
			moveView(m_viewSize / 2 / m_zoomFactor - zoomRect.center() + m_viewDelta);
		}
	}

	void Viewport::setZoom(const types::Point& pos, float factor)
	{
		if (m_zoomFactor == factor)
			return;

		Point oldPos = pos / m_zoomFactor;
		m_zoomFactor = factor;
		m_zoomLevel = static_cast<int>(100.0 * (1.f - m_zoomFactor));
		moveView(pos / m_zoomFactor - oldPos);
	}

	void Viewport::setZoomLevel(const types::Point& pos, int level)
	{
		if (m_zoomLevel == level)
			return;

		Point oldPos = pos / m_zoomFactor;
		m_zoomLevel = level;
		m_zoomFactor = (100 - m_zoomLevel) / 100.f;
		moveView(pos / m_zoomFactor - oldPos);
	}

	void Viewport::setViewSize(types::Point size)
	{
		m_viewSize = size;
	}

	types::Rect Viewport::displayRect() const
	{
		return { m_viewDelta, m_viewDelta + m_viewSize / m_zoomFactor };
	}

	types::Point Viewport::toView(const types::Point& pos) const
	{
		return m_viewDelta + pos / m_zoomFactor;
	}

	types::Point Viewport::fromView(const types::Point& pos) const
	{
		return (pos - m_viewDelta) * m_zoomFactor;
	}
	
} // namespace graphview

} // namespace panda
