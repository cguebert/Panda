#pragma once

#include <panda/graphview/InteractionEvents.h>
#include <panda/graphview/graphics/DrawList.h>

#include <memory>

namespace panda 
{

class BaseData;
class PandaObject;
class ScopedMacro;

namespace graphview
{
	namespace graphics {
		struct DrawColors;
	}

	namespace object {
		class ObjectRenderer;
	}

	class GraphView;
	class LinkTag;
	class SnapToObjects;

	class PANDA_CORE_API ViewInteraction
	{
	public:
		ViewInteraction(GraphView& view);
		virtual ~ViewInteraction();

		void initializeRenderer(ViewRenderer& viewRenderer);
		void beforeDraw();
		void onBeginDraw(const graphics::DrawColors& colors);
		void drawInteraction(graphics::DrawList& drawList, const graphics::DrawColors& colors);
		const std::shared_ptr<graphics::DrawList>& connectedDatasDrawList() const;
		bool highlightConnectedDatas() const;

		virtual void mousePressEvent(const MouseEvent& event);
		virtual void mouseMoveEvent(const MouseEvent& event);
		virtual void mouseReleaseEvent(const MouseEvent& event);
		virtual void wheelEvent(const WheelEvent& event);
		virtual bool keyPressEvent(const KeyEvent& event); // Returns true if the event is used
		virtual void contextMenuEvent(const ContextMenuEvent& event);

		void removeObject(PandaObject* object);
		void clear();

		void moveObjectToBack();
		void moveObjectToFront();
		void removeLink();

		const BaseData* clickedData() const;
		BaseData* contextMenuData() const;

	protected:
		int getContextMenuFlags(const types::Point& pos);

		void moveViewIfMouseOnBorder();
		void updateConnectedDatas(const graphics::DrawColors& colors);

		void hoverDataInfo();

		enum class Moving
		{
			None = 0,
			Start,           // Before the mouse is moved a few pixels
			Object,          // Moving an object
			View,            // Moving the view
			Selection,       // Selection box
			SelectionAdd,    // Selection box, add mode
			SelectionRemove, // Selection box, remove mode
			Link,            // Linking data
			Zoom,            // Zooming with the mouse
			ZoomBox,         // Drawing a zooming box
			Custom
		};
		Moving m_movingAction = Moving::None;

		float m_wheelTicks = 0;

		BaseData* m_clickedData = nullptr;
		BaseData* m_hoverData = nullptr;

		std::shared_ptr<ScopedMacro> m_moveObjectsMacro;

		int m_hoverTimerId = 0; // Reacting when the mouse is staying over a Data
		bool m_highlightConnectedDatas = false;

		std::vector<PandaObject*> m_customSelection; // Objects on which the current action is applied

		PandaObject* m_contextMenuObject = nullptr;
		BaseData* m_contextMenuData = nullptr;
		LinkTag* m_contextLinkTag = nullptr;

		GraphView& m_view;
		long long m_previousTime = 0; // Used for moving the view when the mouse in near a border, while linking data

		types::Point m_previousMousePos, m_currentMousePos;

		object::ObjectRenderer* m_capturedRenderer = nullptr; // Clicked object::ObjectRenderer that want to intercept mouse events

		std::unique_ptr<SnapToObjects> m_snapToObjects;

		bool m_recomputeConnectedDatas = false;
		std::shared_ptr<graphics::DrawList> m_connectedDatasDrawList;
	};

//****************************************************************************//

	inline const BaseData* ViewInteraction::clickedData() const
	{ return m_clickedData; }

	inline const std::shared_ptr<graphics::DrawList>& ViewInteraction::connectedDatasDrawList() const
	{ return m_connectedDatasDrawList; }

	inline bool ViewInteraction::highlightConnectedDatas() const
	{ return m_highlightConnectedDatas; }

} // namespace graphview

} // namespace panda
