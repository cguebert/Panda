#include <ui/graphview/ViewInteraction.h>

#include <ui/command/MoveObjectCommand.h>

#include <ui/graphview/DataLabelAddon.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinksList.h>
#include <ui/graphview/LinkTagsList.h>
#include <ui/graphview/ObjectRenderersList.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/SnapToObjects.h>
#include <ui/graphview/ViewGUI.h>
#include <ui/graphview/Viewport.h>

#include <ui/graphview/graphics/DrawColors.h>
#include <ui/graphview/graphics/DrawList.h>

#include <ui/graphview/object/ObjectRenderer.h>
#include <ui/graphview/object/DockableRenderer.h>

#include <panda/SimpleGUI.h>
#include <panda/TimedFunctions.h>
#include <panda/UndoStack.h>
#include <panda/command/DockableCommand.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>
#include <panda/object/Dockable.h>
#include <panda/types/DataTraits.h>

#include <chrono>

using namespace std::string_literals;

using Point = panda::types::Point;
using Rect = panda::types::Rect;

namespace
{

	long long currentTime()
	{
		using namespace std::chrono;
		auto now = time_point_cast<microseconds>(high_resolution_clock::now());
		return now.time_since_epoch().count();
	}

}

namespace graphview
{

	ViewInteraction::ViewInteraction(GraphView& view)
		: m_view(view)
		, m_snapToObjects(std::make_unique<SnapToObjects>(view))
	{
	}

	ViewInteraction::~ViewInteraction() = default;

	void ViewInteraction::beforeDraw()
	{
		// Moving the view when creating a link if the mouse is near the border of the widget
		if (m_movingAction == Moving::Link)
			moveViewIfMouseOnBorder();
	}

	void ViewInteraction::onBeginDraw(const graphics::DrawColors& colors)
	{
		if(m_recomputeConnectedDatas)
			updateConnectedDatas(colors);
	}

	void ViewInteraction::drawInteraction(graphics::DrawList& drawList, const graphics::DrawColors& colors)
	{
		const auto zoom = m_view.viewport().zoom();
		const auto viewDelta = m_view.viewport().viewDelta();

		// Selection rubber band
		if (m_movingAction == Moving::Selection
			|| m_movingAction == Moving::SelectionAdd
			|| m_movingAction == Moving::SelectionRemove)
		{
			auto r = Rect(m_previousMousePos / zoom, m_currentMousePos / zoom).translated(viewDelta).canonicalized();
			auto highlight = colors.highlightColor;
			highlight = (highlight & 0x00FFFFFF) | 0x40000000;
			drawList.addRectFilled(r, highlight);
			drawList.addRect(r, colors.penColor, 0.75f / zoom);
		}

		// Zoom box
		if (m_movingAction == Moving::ZoomBox)
		{
			auto r = Rect(m_previousMousePos / zoom, m_currentMousePos / zoom).translated(viewDelta).canonicalized();
			drawList.addRect(r, colors.penColor, 0.75f / zoom);
		}

		// Link in creation
		if (m_movingAction == Moving::Link)
		{
			drawList.addLine(m_previousMousePos, m_currentMousePos, colors.penColor, 1.5);
		}
	}

	void ViewInteraction::mousePressEvent(const MouseEvent& event)
	{
		m_previousTime = currentTime();
		Point zoomedMouse = m_view.viewport().toView(event.pos());
		if (event.button() == MouseButton::LeftButton)
		{
			// Testing for Datas first
			const auto dataRect = m_view.linksList().getDataAtPos(zoomedMouse);
			if (dataRect.first)
			{
				if (!m_view.document()->animationIsPlaying())
				{
					const auto data = dataRect.first;
					// Remove
					if (data->isInput() && data->getParent() && event.modifiers() == EventModifier::ControlModifier)
					{
						m_view.linkTagsList().removeLinkTag(data->getParent(), data);
						m_view.linksList().changeLink(data, nullptr);
					}
					else	// Creating a new Link
					{
						m_clickedData = data;
						m_view.linksList().computeCompatibleDatas(data);
						m_movingAction = Moving::Link;
						m_previousMousePos = m_currentMousePos = dataRect.second.center();

						m_view.selection().selectNone();
					}
				}
			}
			else if (const auto objRnd = m_view.objectRenderers().getAtPos(zoomedMouse))
			{ // No Data, but we still clicked on an object
				auto object = objRnd->getObject();

				// Add the object to the selection
				if (event.modifiers() == EventModifier::ControlModifier)
				{
					if (m_view.selection().isSelected(object))
						m_view.selection().remove(object);
					else
						m_view.selection().add(object);
				}
				else	// Moving the object (or selecting only this one if we release the mouse without moving)
				{
					if (!m_view.selection().isSelected(object))
						m_view.selection().selectOne(object);
					else
						m_view.selection().setLastSelectedObject(object);
					m_movingAction = Moving::Start;
					m_previousMousePos = zoomedMouse;
				}

				// Maybe do a custom action ?
				if (objRnd->mousePressEvent(event))
				{
					m_movingAction = Moving::Custom;
					m_capturedRenderer = objRnd;
				}
			}
			else
			{	// Clicked where there is nothing
				const auto modifiers = event.modifiers();
				if (modifiers == EventModifier::NoModifier)
				{
					m_movingAction = Moving::Selection; // Starting a rubber band to select in a zone
					m_view.selection().selectNone();
				}
				else if (modifiers & EventModifier::ShiftModifier)
				{
					if (modifiers & EventModifier::ControlModifier)
						m_movingAction = Moving::SelectionRemove;
					else
						m_movingAction = Moving::SelectionAdd;
				}
				else if (event.modifiers() == EventModifier::ControlModifier)
					m_movingAction = Moving::ZoomBox; // Starting a zoom box

				m_previousMousePos = m_currentMousePos = event.pos();
				m_view.gui().setCursor(Cursor::Cross);
			}
		}
		else if (event.button() == MouseButton::MiddleButton)
		{
			if (event.modifiers() == EventModifier::ControlModifier)
			{
				m_movingAction = Moving::Zoom;
				m_currentMousePos = event.pos();
				m_previousMousePos = event.pos();

				m_view.gui().setCursor(Cursor::SizeVer);

			}
			else
			{
				m_movingAction = Moving::View;
				m_previousMousePos = event.pos();

				m_view.gui().setCursor(Cursor::SizeAll);
			}
		}
	}

	void ViewInteraction::mouseMoveEvent(const MouseEvent& event)
	{
		if (m_movingAction == Moving::Start)
		{
			Point mousePos = m_view.viewport().toView(event.pos());
			Point delta = mousePos - m_previousMousePos;
			if ((delta * m_view.viewport().zoom()).norm() > 5)
			{
				m_movingAction = Moving::Object;
				if (!m_view.selectedObjectsRenderers().empty())
				{
					auto objRnd = m_view.selectedObjectsRenderers().back();
					if (objRnd->acceptsMagneticSnap())
					{
						m_snapToObjects->prepareSnapTargets(objRnd);
						auto possiblePosition = objRnd->getPosition() + delta;
						m_snapToObjects->updateSnapDelta(objRnd, possiblePosition);
						delta += m_snapToObjects->snapDelta();
					}
				}
				m_view.gui().setCursor(Cursor::SizeAll);

				// Make sure docked objects are in the selection with their dock
				std::set<panda::PandaObject*> selectionSet;
				for (auto object : m_view.selection().get())
				{
					selectionSet.insert(object);
					panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object);
					if (dock)
					{
						for (auto docked : dock->getDockedObjects())
							selectionSet.insert(docked);
					}
				}

				m_customSelection.clear();
				for (auto object : selectionSet)
					m_customSelection.push_back(object);

				m_moveObjectsMacro = m_view.document()->getUndoStack().beginMacro("move objects");

				if (!delta.isNull())
					m_view.document()->getUndoStack().push(std::make_shared<MoveObjectCommand>(m_customSelection, delta));

				m_previousMousePos = mousePos;
			}
		}
		else if (m_movingAction == Moving::Object)
		{
			Point mousePos = m_view.viewport().toView(event.pos());
			Point delta = mousePos - m_previousMousePos;
			if (!m_view.selectedObjectsRenderers().empty())
			{
				Point oldSnapDelta = m_snapToObjects->snapDelta();
				auto objRnd = m_view.selectedObjectsRenderers().back();
				auto possiblePosition = objRnd->getPosition() + delta - oldSnapDelta;
				m_snapToObjects->updateSnapDelta(objRnd, possiblePosition);
				delta = delta - oldSnapDelta + m_snapToObjects->snapDelta();
			}

			if (!m_customSelection.empty() && !delta.isNull())
			{
				m_view.document()->getUndoStack().push(std::make_shared<MoveObjectCommand>(m_customSelection, delta));
				m_view.linksList().clear();
			}

			m_previousMousePos = mousePos;
		}
		else if (m_movingAction == Moving::View)
		{
			m_view.viewport().moveView((event.pos() - m_previousMousePos) / m_view.viewport().zoom());
			m_previousMousePos = event.pos();
		}
		else if (m_movingAction == Moving::Zoom)
		{
			int y = event.pos().y - m_previousMousePos.y;
			auto zoom = panda::helper::bound(0.1f, m_view.viewport().zoom() - y / 500.0f, 1.0f);
			m_view.viewport().setZoom(m_currentMousePos, zoom);
			m_previousMousePos = event.pos();
		}
		else if (m_movingAction == Moving::Selection
				 || m_movingAction == Moving::SelectionAdd
				 || m_movingAction == Moving::SelectionRemove
				 || m_movingAction == Moving::ZoomBox)
		{
			m_currentMousePos = event.pos();
			m_view.update();
		}
		else if (m_movingAction == Moving::Link)
		{
			m_currentMousePos = m_view.viewport().toView(event.pos());

			auto dataRect = m_view.linksList().getDataAtPos(m_currentMousePos);
			if (dataRect.first && m_view.linksList().canLinkWith(dataRect.first))
				m_currentMousePos = dataRect.second.center();

			// Moving the view if the mouse is near the border of the widget
			moveViewIfMouseOnBorder();

			m_view.update();
		}
		else if (m_movingAction == Moving::Custom)
		{
			if (m_capturedRenderer)
				m_capturedRenderer->mouseMoveEvent(event);
		}

		if (m_movingAction == Moving::None || m_movingAction == Moving::Link)
		{
			Point zoomedMouse = m_view.viewport().toView(event.pos());
			const auto dataRect = m_view.linksList().getDataAtPos(zoomedMouse);
			if (dataRect.first)
			{
				if (m_hoverData != dataRect.first)
				{
					auto& timedFunctions = panda::TimedFunctions::instance();
					timedFunctions.cancelRun(m_hoverTimerId);
					if (m_highlightConnectedDatas)
					{
						m_highlightConnectedDatas = false;
						m_view.update();
					}
					m_hoverData = dataRect.first;
					m_hoverTimerId = timedFunctions.delayRun(0.5, [this] { hoverDataInfo(); });
				}

				auto label = DataLabelAddon::getDataLabel(m_hoverData);

				std::string display;
				if (!label.empty())
					display = "<b>"s + label + "</b>"s;
				else
					display = m_hoverData->getName() + "\n" + m_hoverData->getDescription();

				Rect area { m_view.toScreen({dataRect.second.topLeft()}), m_view.toScreen({dataRect.second.bottomRight()}) };
				m_view.gui().showToolTip(m_view.toScreen(event.pos()), display, area);
				if (!m_hoverData->getHelp().empty())
					m_view.gui().setStatusBarMessage(m_hoverData->getHelp());
			}
			else
			{
				m_hoverData = nullptr;
				if (m_highlightConnectedDatas)
				{
					m_highlightConnectedDatas = false;
					m_view.update();
				}
				panda::TimedFunctions::instance().cancelRun(m_hoverTimerId);

				if (m_movingAction == Moving::None)
				{
					// Look for link tags
					for (auto& tag : m_view.linkTagsList().get())
					{
						auto dataPair = tag->getDataAtPoint(zoomedMouse);
						auto data = dataPair.first;
						bool hovering = data != nullptr;
						if (hovering != tag->isHovering())
						{
							tag->setHovering(hovering);
							m_view.update();
						}

						if (!data)
							continue;

						auto label = DataLabelAddon::getDataLabel(tag->getInputData());

						std::string display;
						if (!label.empty())
							display = "<b>"s + label + "</b>"s;
						else if (data && data->isInput())
						{
							auto parent = data->getParent();
							display = parent->getOwner()->getName() + "\n" + parent->getName();
						}

						auto tagRect = dataPair.second;
						Rect area { m_view.toScreen({tagRect.topLeft()}), m_view.toScreen({tagRect.bottomRight()}) };
						m_view.gui().showToolTip(m_view.toScreen(event.pos()), display, area);
					}
				}
			}
		}
	}

	void ViewInteraction::mouseReleaseEvent(const MouseEvent& event)
	{
		if (m_movingAction == Moving::Start)
		{
			panda::PandaObject* object = m_view.selection().lastSelectedObject();
			if (object)
				m_view.selection().selectOne(object);
		}
		else if (m_movingAction == Moving::Object)
		{
			std::map<panda::PandaObject*, Point> positions;
			for (const auto objRnd : m_view.selectedObjectsRenderers())
				positions[objRnd->getObject()] = objRnd->getPosition();

			for (const auto objRnd : m_view.selectedObjectsRenderers())
			{
				auto object = objRnd->getObject();
				panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
				if (dockable && !m_view.selection().isSelected(dockable->getParentDock()))
				{
					Point delta = positions[object] - objRnd->getPosition();
					m_view.document()->getUndoStack().push(std::make_shared<MoveObjectCommand>(dockable, delta));

					Rect dockableArea = objRnd->getSelectionArea();
					panda::DockObject* defaultDock = dockable->getDefaultDock();
					panda::DockObject* newDock = defaultDock;
					int newIndex = -1;
					for (const auto objRnd2 : m_view.objectRenderers().getOrdered())
					{
						panda::DockObject* dock = dynamic_cast<panda::DockObject*>(objRnd2->getObject());
						if (dock)
						{
							if (dockableArea.intersects(objRnd2->getSelectionArea()) && dock->accepts(dockable))
							{
								newIndex = dynamic_cast<object::DockObjectRenderer*>(objRnd2)->getDockableIndex(dockableArea);
								newDock = dock;
								break;
							}
						}
					}

					panda::DockObject* prevDock = dockable->getParentDock();
					if (newDock != prevDock) // Changing dock
					{
						if (prevDock)
							m_view.document()->getUndoStack().push(std::make_shared<panda::DetachDockableCommand>(prevDock, dockable));
						if (newDock)
						{
							m_view.document()->getUndoStack().push(std::make_shared<panda::AttachDockableCommand>(newDock, dockable, newIndex));
							m_view.document()->onChangedDock(dockable);
						}
					}
					else if (prevDock != defaultDock) // (maybe) Changing place in the dock
					{
						int prevIndex = prevDock->getIndexOfDockable(dockable);
						if (prevIndex != newIndex)
						{
							if (newIndex > prevIndex)
								--newIndex;

							m_view.document()->getUndoStack().push(std::make_shared<panda::ReorderDockableCommand>(prevDock, dockable, newIndex));
						}
						m_view.modifiedObject(prevDock); // Always update
					}
					else if (defaultDock) // (maybe) Changing place in the default dock
						m_view.sortDockable(dockable, defaultDock);
				}
			}

			m_moveObjectsMacro.reset();

			m_view.viewport().updateObjectsRect();
		}
		else if (m_movingAction == Moving::View)
		{
			m_view.emitViewportModified();
		}
		else if (m_movingAction == Moving::Zoom)
		{
			m_view.viewport().updateViewRect();
		}
		else if (m_movingAction == Moving::ZoomBox)
		{
			m_view.viewport().setViewport({ m_previousMousePos, m_currentMousePos });
		}
		else if (m_movingAction == Moving::Selection
				 || m_movingAction == Moving::SelectionAdd
				 || m_movingAction == Moving::SelectionRemove)
		{
			const auto zoom = m_view.viewport().zoom();
			bool remove = m_movingAction == Moving::SelectionRemove;
			ObjectsSelection::Objects selection = m_view.selection().get();
			Rect selectionRect = Rect(m_previousMousePos / zoom, m_currentMousePos / zoom)
				.translated(m_view.viewport().viewDelta())
				.canonicalized();
			for (const auto objRnd : m_view.objectRenderers().getOrdered())
			{
				Rect objectArea = objRnd->getSelectionArea();
				if (selectionRect.intersects(objectArea))
				{
					auto object = objRnd->getObject();
					if (remove)
						panda::helper::removeOne(selection, object);
					else if (!panda::helper::contains(selection, object))
						selection.push_back(objRnd->getObject());
				}
			}

			m_view.selection().set(selection);
			m_view.update();
		}
		else if (m_movingAction == Moving::Link)
		{
			const auto dataRect = m_view.linksList().getDataAtPos(m_currentMousePos);
			auto data = dataRect.first;
			if (data && m_view.linksList().canLinkWith(data))
			{
				if (m_view.linksList().createLink(m_clickedData, data))
					m_view.linkTagsList().setDirty();
			}
			m_clickedData = nullptr;
			m_view.update();
		}
		else if (m_movingAction == Moving::Custom)
		{
			if (m_capturedRenderer)
			{
				m_capturedRenderer->mouseReleaseEvent(event);
				m_capturedRenderer = nullptr;
				m_view.viewport().updateObjectsRect();
			}
		}

		m_view.gui().restoreCursor();
		m_movingAction = Moving::None;
	}

	void ViewInteraction::wheelEvent(const WheelEvent& event)
	{
		if (m_movingAction != Moving::None)
			return;

		m_wheelTicks += event.angleDelta().y;
		int ticks = m_wheelTicks / 40; // Steps of 5 degrees
		m_wheelTicks -= ticks * 40;
		const auto zoomLevel = m_view.viewport().zoomLevel();
		int newZoom = panda::helper::bound(0, zoomLevel - ticks, 90);
		m_view.viewport().setZoomLevel(event.pos(), newZoom);
	}

	bool ViewInteraction::keyPressEvent(const KeyEvent& event)
	{
		switch (event.key())
		{
		case Key::Left:
			if (event.modifiers() & EventModifier::ControlModifier)
				m_view.viewport().moveView({ 100, 0 });
			return true;
		case Key::Right:
			if (event.modifiers() & EventModifier::ControlModifier)
				m_view.viewport().moveView({ -100, 0 });
			return true;
		case Key::Up:
			if (event.modifiers() & EventModifier::ControlModifier)
				m_view.viewport().moveView({ 0, 100 });
			return true;
		case Key::Down:
			if (event.modifiers() & EventModifier::ControlModifier)
				m_view.viewport().moveView({ 0, -100 });
			return true;
		case Key::Plus:
			if (event.modifiers() & EventModifier::ControlModifier)
				m_view.viewport().zoomIn();
			return true;
		case Key::Minus:
			if (event.modifiers() & EventModifier::ControlModifier)
				m_view.viewport().zoomOut();
			return true;
		default:
			return false;
		}
	}

	void ViewInteraction::contextMenuEvent(const ContextMenuEvent& event)
	{
		m_contextMenuData = nullptr;

		Point pos = m_view.viewport().toView(event.pos());
		int flags = getContextMenuFlags(pos);

		panda::TimedFunctions::instance().cancelRun(m_hoverTimerId);

		const auto gPos = m_view.toScreen(event.pos());
		const auto posI = panda::graphics::PointInt(static_cast<int>(gPos.x), static_cast<int>(gPos.y));
		m_view.document()->getGUI().contextMenu(posI, flags);
	}

	panda::BaseData* ViewInteraction::contextMenuData() const
	{ 
		return m_contextLinkTag ? m_contextLinkTag->getInputData() : m_contextMenuData; 
	}

	int ViewInteraction::getContextMenuFlags(const panda::types::Point& pos)
	{
		namespace gm = panda::gui::menu;
		int flags = gm::Selection; // Let MainWindow fill the menu based on the current selection
		const auto objRnd = m_view.objectRenderers().getAtPos(pos);
		if (objRnd)
		{
			m_contextMenuObject = objRnd->getObject();
			flags |= gm::Object;
			m_contextMenuData = objRnd->getDataAtPos(pos);
			if (m_contextMenuData)
			{
				if (m_contextMenuData->isDisplayed())
					flags |= gm::Data;

				if (m_contextMenuData->isInput() && m_contextMenuData->getParent())
					flags |= gm::Link;

				const auto trait = m_contextMenuData->getDataTrait();
				if (trait->valueTypeName() == "image")
					flags |= gm::Image;
			}
		}
		else
			m_contextMenuObject = nullptr;

		m_contextLinkTag = nullptr;
		for (const auto& linkTag : m_view.linkTagsList().get())
		{
			auto dataPair = linkTag->getDataAtPoint(pos);
			if (dataPair.first)
			{
				flags |= gm::Tag;
				m_contextLinkTag = linkTag.get();
				break;
			}
		}

		return flags;
	}

	void ViewInteraction::moveViewIfMouseOnBorder()
	{
		auto pos = m_view.viewport().fromView(m_currentMousePos);
		auto area = m_view.contentsArea();
		const float maxDist = 50;
		area.adjust(maxDist, maxDist, -maxDist, -maxDist);
		if (!area.contains(pos))
		{
			float dx = 0, dy = 0;
			if (pos.x < area.left())
				dx = area.left() - pos.x;
			else if (pos.x > area.right())
				dx = area.right() - pos.x;

			if (pos.y < area.top())
				dy = area.top() - pos.y;
			else if (pos.y > area.bottom())
				dy = area.bottom() - pos.y;

			auto now = currentTime();
			if (now > m_previousTime)
			{
				float dt = (now - m_previousTime) / 1000000.f;
				const float speed = 10.f / m_view.viewport().zoom();
				m_previousTime = now;
				Point delta = speed * dt * Point(dx, dy);
				m_view.viewport().moveView(delta);
			}

			m_view.update();
		}
	}

	void ViewInteraction::updateConnectedDatas(const graphics::DrawColors& colors)
	{
		m_recomputeConnectedDatas = false;
		
		if (!m_highlightConnectedDatas)
			return;

		m_connectedDatasDrawList = {};

		auto connected = m_view.linksList().getConnectedDatas(m_hoverData);
		const auto& rects = connected.first;
		const auto& links = connected.second;

		if(links.empty())
			return;

		// Now draw everything
		for (const auto& rect : rects)
		{
			m_connectedDatasDrawList.addRectFilled(rect, colors.highlightColor);
			m_connectedDatasDrawList.addRect(rect, colors.penColor, 1.f);
		}

		for(const auto& link : links)
		{
			float w = (link.second.x - link.first.x) / 2;
			auto p1 = link.first, p2 = link.second, d = Point(w, 0);
			m_connectedDatasDrawList.addBezierCurve(p1, p1 + d, p2 - d, p2, colors.highlightColor, 3);
		}
	}

	void ViewInteraction::removeObject(panda::PandaObject* object)
	{
		m_movingAction = Moving::None;
		m_capturedRenderer = nullptr;
		m_highlightConnectedDatas = false;
	}

	void ViewInteraction::clear()
	{
		m_recomputeConnectedDatas = true;
		m_hoverData = nullptr;
		m_highlightConnectedDatas = false;
		panda::TimedFunctions::instance().cancelRun(m_hoverTimerId);
	}

	void ViewInteraction::moveObjectToBack()
	{
		if(m_contextMenuObject)
			m_view.objectsList().reinsertObject(m_contextMenuObject, 0); // Front of the list = others are drawn on top
	}

	void ViewInteraction::moveObjectToFront()
	{
		if(m_contextMenuObject)
			m_view.objectsList().reinsertObject(m_contextMenuObject, -1); // Back of the list = drawn on top of the others
	}

	void ViewInteraction::removeLink()
	{
		if(m_contextMenuData && m_contextMenuData->isInput() && m_contextMenuData->getParent())
		{
			m_view.linkTagsList().removeLinkTag(m_contextMenuData->getParent(), m_contextMenuData);
			m_view.linksList().changeLink(m_contextMenuData, nullptr);
			m_contextMenuData = nullptr;
			m_view.update();
		}
	}

	void ViewInteraction::hoverDataInfo()
	{
		if(m_hoverData)
		{
			m_highlightConnectedDatas = true;
			m_recomputeConnectedDatas = true;
			m_view.update();
		}
	}

} // namespace graphview
