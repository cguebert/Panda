#pragma once

#include <panda/messaging.h>
#include <panda/types/Rect.h>

namespace panda
{

namespace graphview
{

	class GraphView;

	class PANDA_CORE_API Viewport
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

		void moveView(const types::Point& delta);

		virtual void updateObjectsRect();
		void updateViewRect();

		void setViewport(const types::Rect& globalArea); // Given in non-transformed coordinates

		float zoom() const;
		void setZoom(const types::Point& pos, float zoomFactor); // The position will stay put while the zoom changes

		int zoomLevel() const;
		void setZoomLevel(const types::Point& pos, int zoomLevel); // The position will stay put while the zoom changes

		types::Point viewSize() const; // The size of the view widget
		void setViewSize(types::Point size); // Set by the graph view widget

		types::Point viewDelta() const;
		types::Rect displayRect() const; // The area that is shown
		const types::Rect& objectsRect() const; // The area taken by the objects
		const types::Rect& viewRect() const; // The area taken by the objects, with the zoom

		types::Point toView(const types::Point& pos) const;
		types::Point fromView(const types::Point& pos) const;

		msg::Signal<void()> modified;

	protected:
		GraphView& m_view;
		int m_zoomLevel = 0;
		float m_zoomFactor = 1.0f;
		types::Point m_viewSize = { 800, 600 };
		types::Point m_viewDelta;
		types::Rect m_objectsRect; // Area taken by the objects on the screen
		types::Rect m_viewRect; // Area taken by the objects on the screen, including zoom
	};

//****************************************************************************//

	inline float Viewport::zoom() const
	{ return m_zoomFactor; }

	inline int Viewport::zoomLevel() const
	{ return m_zoomLevel; }

	inline types::Point Viewport::viewSize() const
	{ return m_viewSize; }

	inline types::Point Viewport::viewDelta() const
	{ return m_viewDelta; }

	inline const types::Rect& Viewport::viewRect() const
	{ return m_viewRect; }

	inline const types::Rect& Viewport::objectsRect() const
	{ return m_objectsRect; }

} // namespace graphview

} // namespace panda
