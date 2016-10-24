#pragma once

#include <panda/messaging.h>
#include <panda/types/Rect.h>

namespace graphview
{

	class GraphView;

	class Viewport
	{
	public:
		Viewport(GraphView& view);
		virtual ~Viewport();

		void zoomIn();
		void zoomOut();
		void zoomReset();
		void centerView();
		void showAll();
		void showAllSelected();
		void moveSelectedToCenter();

		void moveView(const panda::types::Point& delta);

		virtual void updateObjectsRect();
		void updateViewRect();

		void setViewport(const panda::types::Rect& globalArea); // Given in non-transformed coordinates

		float zoom() const;
		void setZoom(const panda::types::Point& pos, float zoomFactor); // The position will stay put while the zoom changes

		int zoomLevel() const;
		void setZoomLevel(const panda::types::Point& pos, int zoomLevel); // The position will stay put while the zoom changes

		panda::types::Point viewDelta() const;
		panda::types::Rect displayRect() const; // The area that is shown
		const panda::types::Rect& objectsRect() const; // The area taken by the objects
		const panda::types::Rect& viewRect() const; // The area taken by the objects, with the zoom

		panda::types::Point toView(const panda::types::Point& pos) const;
		panda::types::Point fromView(const panda::types::Point& pos) const;

		panda::msg::Signal<void()> modified;

	protected:
		GraphView& m_view;
		int m_zoomLevel = 0;
		float m_zoomFactor = 1.0f;
		panda::types::Point m_viewDelta;
		panda::types::Rect m_objectsRect; // Area taken by the objects on the screen
		panda::types::Rect m_viewRect; // Area taken by the objects on the screen, including zoom
	};

//****************************************************************************//

	inline float Viewport::zoom() const
	{ return m_zoomFactor; }

	inline int Viewport::zoomLevel() const
	{ return m_zoomLevel; }

	inline panda::types::Point Viewport::viewDelta() const
	{ return m_viewDelta; }

	inline const panda::types::Rect& Viewport::viewRect() const
	{ return m_viewRect; }

	inline const panda::types::Rect& Viewport::objectsRect() const
	{ return m_objectsRect; }

}
