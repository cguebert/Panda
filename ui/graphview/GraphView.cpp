#include <QtWidgets>
#include <chrono>
#include <functional>
#include <limits>

#include <ui/command/MoveObjectCommand.h>
#include <ui/dialog/ChooseWidgetDialog.h>
#include <ui/dialog/QuickCreateDialog.h>
#include <ui/graphview/object/ObjectRenderer.h>
#include <ui/graphview/object/DockableRenderer.h>
#include <ui/graphview/DataLabelAddon.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinkTag.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/ObjectRenderersList.h>
#include <ui/graphview/Viewport.h>
#include <ui/graphview/ViewRenderer.h>
#include <ui/graphview/graphics/DrawList.h>

#include <panda/document/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/helper/algorithm.h>
#include <panda/types/DataTraits.h>
#include <panda/command/AddObjectCommand.h>
#include <panda/command/DockableCommand.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/command/RemoveObjectCommand.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/ObjectsList.h>
#include <panda/document/Serialization.h>

#ifdef PANDA_LOG_EVENTS
#include <ui/dialog/UpdateLoggerDialog.h>
#endif

using Point = panda::types::Point;
using Rect = panda::types::Rect;

namespace
{
	inline panda::types::Point convert(const QPointF& pt)
	{ return { static_cast<float>(pt.x()), static_cast<float>(pt.y()) }; }

	inline panda::types::Rect convert(const QRect& r)
	{ return panda::types::Rect(r.left(), r.top(), r.right(), r.bottom()); }

	inline QPoint convert(const panda::types::Point& pt)
	{ return QPointF{ pt.x, pt.y }.toPoint(); }

	long long currentTime()
	{
		using namespace std::chrono;
		auto now = time_point_cast<microseconds>(high_resolution_clock::now());
		return now.time_since_epoch().count();
	}
}

namespace graphview
{

GraphView::GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent)
	: QOpenGLWidget(parent)
	, m_pandaDocument(doc)
	, m_objectsList(objectsList)
	, m_hoverTimer(new QTimer(this))
	, m_objectsSelection(std::make_unique<ObjectsSelection>(objectsList))
	, m_viewRenderer(std::make_unique<ViewRenderer>())
	, m_objectRenderersList(std::make_unique<ObjectRenderersList>())
	, m_viewport(std::make_unique<Viewport>(*this))
{
	QSurfaceFormat fmt;
	fmt.setSamples(8);
	setFormat(fmt);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	m_observer.get(m_objectsSelection->selectionChanged).connect<GraphView, &GraphView::selectionChanged>(this);

	auto& docSignals = m_pandaDocument->getSignals();
	m_observer.get(docSignals.modified).connect<QWidget, &QWidget::update>(this);
	m_observer.get(docSignals.modifiedObject).connect<GraphView, &GraphView::modifiedObject>(this);
	m_observer.get(docSignals.startLoading).connect<GraphView, &GraphView::startLoading>(this);
	m_observer.get(docSignals.loadingFinished).connect<GraphView, &GraphView::loadingFinished>(this);
	m_observer.get(docSignals.changedDock).connect<GraphView, &GraphView::changedDock>(this);

	m_observer.get(m_objectsList.addedObject).connect<GraphView, &GraphView::addedObject>(this);
	m_observer.get(m_objectsList.removedObject).connect<GraphView, &GraphView::removeObject>(this);
	m_observer.get(m_objectsList.reorderedObjects).connect<GraphView, &GraphView::objectsReordered>(this);

	m_observer.get(m_viewport->modified).connect<QWidget, &QWidget::update>(this);
	m_observer.get(m_viewport->modified).connect<GraphView, &GraphView::emitViewportModified>(this);

	connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverDataInfo()));

	m_hoverTimer->setSingleShot(true);

	setMouseTracking(true);

	const auto& pal = palette();
	m_drawColors.penColor = graphics::DrawList::convert(pal.text().color());
	m_drawColors.midLightColor = graphics::DrawList::convert(pal.midlight().color());
	m_drawColors.lightColor = graphics::DrawList::convert(pal.light().color());
	m_drawColors.highlightColor = graphics::DrawList::convert(pal.highlight().color());

	// Create the draw structs for the objects already present
	for (const auto& object : m_objectsList.get())
		addedObject(object.get());

	updateDirtyRenderers();
}

GraphView::~GraphView() = default;

QSize GraphView::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize GraphView::sizeHint() const
{
	return QSize(600, 400);
}

std::pair<panda::BaseData*, Rect> GraphView::getDataAtPos(const panda::types::Point& pt)
{
	const auto objRnd = objectRenderers().getAtPos(pt);
	if (objRnd)
	{
		panda::BaseData* data = objRnd->getDataAtPos(pt);
		Rect dataRect;
		if (objRnd->getDataRect(data, dataRect))
			return{ data, dataRect };
	}

	return{ nullptr, Rect() };
}

bool GraphView::getDataRect(const panda::BaseData* data, panda::types::Rect& rect)
{
	auto objRnd = objectRenderers().get(data->getOwner());
	if (!objRnd)
		return false;
	return objRnd->getDataRect(data, rect);
}

void GraphView::initializeGL()
{
	m_viewRenderer->initialize();
	m_linksDrawList = {};
	m_connectedDrawList = {};
}

void GraphView::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	m_viewRenderer->resize(w, h);
	update();
}

void GraphView::paintGL()
{
	decltype(m_functionsToExecuteNextRefresh) functions;
	functions.swap(m_functionsToExecuteNextRefresh);
	for (const auto func : functions)
		func();
	
	// Moving the view when creating a link if the mouse is near the border of the widget
	if(m_movingAction == Moving::Link)
		moveViewIfMouseOnBorder();

	updateDirtyRenderers();

	if (m_objectsMoved)
	{
		m_objectsMoved = false;
		emit modified();
		m_recomputeTags = true;
		m_recomputeLinks = true;
		viewport().updateObjectsRect();
	}

	if(m_recomputeTags)			updateLinkTags();
	if(m_recomputeLinks)		updateLinks();
	if(m_recomputeConnected)	updateConnectedDatas();

	const auto displayRect = viewport().displayRect();
	m_viewRenderer->setView(displayRect);
	m_viewRenderer->newFrame();
	graphics::DrawList drawList;

	auto col = palette().background().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const auto& orderedObjectRenderers = objectRenderers().getOrdered();

	// Give a possibility to draw behind normal objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->drawBackground(drawList, m_drawColors);
	}

	// Draw links
	drawList.merge(m_linksDrawList);

	// Draw the objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->draw(drawList, m_drawColors);
	}

	// Give a possibility to draw in front of normal objects
	for (auto& objRnd : orderedObjectRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->drawForeground(drawList, m_drawColors);
	}

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for (auto& objRnd : m_selectedObjectsRenderers)
	{
		if (objRnd->getVisualArea().intersects(displayRect))
			objRnd->draw(drawList, m_drawColors, true);
	}

	// Draw links tags
	for (auto& tag : m_linkTags)
		tag->draw(drawList, m_drawColors);

	const auto zoom = viewport().zoom();
	const auto viewDelta = viewport().viewDelta();

	// Selection rubber band
	if (m_movingAction == Moving::Selection 
		|| m_movingAction == Moving::SelectionAdd 
		|| m_movingAction == Moving::SelectionRemove)
	{
		auto r = Rect(m_previousMousePos / zoom, m_currentMousePos / zoom).translated(viewDelta).canonicalized();
		auto highlight = m_drawColors.highlightColor;
		highlight = (highlight & 0x00FFFFFF) | 0x40000000;
		drawList.addRectFilled(r, highlight);
		drawList.addRect(r, m_drawColors.penColor, 0.75f / zoom);
	}

	// Zoom box
	if (m_movingAction == Moving::ZoomBox)
	{
		auto r = Rect(m_previousMousePos / zoom, m_currentMousePos / zoom).translated(viewDelta).canonicalized();
		drawList.addRect(r, m_drawColors.penColor, 0.75f / zoom);
	}

	// Link in creation
	if (m_movingAction == Moving::Link)
	{
		drawList.addLine(m_previousMousePos, m_currentMousePos, m_drawColors.penColor, 1.5);
	}

	if (m_debugDirtyState)
	{
#ifdef PANDA_LOG_EVENTS
		UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
		if(logDlg && logDlg->isVisible())
			paintLogDebug(drawList, m_drawColors);
		else
#endif
			paintDirtyState(drawList, m_drawColors);
	}

	// Add the main draw list
	m_viewRenderer->addDrawList(&drawList);

	// Highlight connected Datas
	if(m_highlightConnectedDatas)
		m_viewRenderer->addDrawList(&m_connectedDrawList);

	// Execute the render commands
	m_viewRenderer->render();
}

#ifdef PANDA_LOG_EVENTS
void GraphView::paintLogDebug(graphics::DrawList& list, graphics::DrawColors& colors)
{
	UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
	if(logDlg && logDlg->isVisible())
	{
		const auto& states = logDlg->getNodeStates();
		for(const auto objRnd : objectRenderers().getOrdered())
		{
			const auto object = objRnd->getObject();
			unsigned int fillCol = panda::helper::valueOrDefault(states, object, nullptr) ? 0x200000FF : 0x2000FF00;
	
			auto area = objRnd->getVisualArea();
			list.addRectFilled(area, fillCol);

			for(panda::BaseData* data : object->getDatas())
			{
				if(objRnd->getDataRect(data, area))
				{
					fillCol = panda::helper::valueOrDefault(states, data, nullptr) ? 0x400000FF : 0x4000FF00;
					list.addRectFilled(area, fillCol);
				}
			}
		}

		const panda::helper::EventData* event = logDlg->getSelectedEvent();
		if(event)
		{
			panda::PandaObject* object = m_objectsList.find(event->m_objectIndex);
			if(object)
			{
				auto objRnd = objectRenderers().get(object);
				Rect area;

				bool drawData = false;
				const panda::BaseData* data = dynamic_cast<const panda::BaseData*>(event->m_node);
				if(data)
					drawData = objRnd->getDataRect(data, area);
				if(!drawData)
					area = objRnd->getVisualArea();

				list.addRectFilled(area, 0x80FF8080);
			}
		}
	}
}
#endif

void GraphView::paintDirtyState(graphics::DrawList& list, graphics::DrawColors& colors)
{
	for(const auto& objRnd : objectRenderers().getOrdered())
	{
		const auto object = objRnd->getObject();
		unsigned int fillCol = object->isDirty() ? 0x400000FF : 0x4000FF00;

		auto area = objRnd->getVisualArea();
		list.addRectFilled(area, fillCol);

		for(panda::BaseData* data : object->getDatas())
		{
			Rect area;
			if(objRnd->getDataRect(data, area))
			{
				fillCol = data->isDirty() ? 0x400000FF : 0x4000FF00;
				list.addRectFilled(area, fillCol);
			}
		}
	}
}

void GraphView::mousePressEvent(QMouseEvent* event)
{
	m_previousTime = currentTime();
	Point localPos = convert(event->localPos());
	Point zoomedMouse = viewport().toView(localPos);
	if(event->button() == Qt::LeftButton)
	{
		// Testing for Datas first
		const auto dataRect = getDataAtPos(zoomedMouse);
		if (dataRect.first)
		{
			if (!m_pandaDocument->animationIsPlaying())
			{
				const auto data = dataRect.first;
				// Remove
				if (data->isInput() && data->getParent() && event->modifiers() == Qt::ControlModifier)
				{
					removeLinkTag(data->getParent(), data);
					changeLink(data, nullptr);
				}
				else	// Creating a new Link
				{
					m_clickedData = data;
					computeCompatibleDatas(data);
					m_movingAction = Moving::Link;
					m_previousMousePos = m_currentMousePos = dataRect.second.center();

					m_objectsSelection->selectNone();
				}
			}
		}
		else if(const auto objRnd = objectRenderers().getAtPos(zoomedMouse))
		{ // No Data, but we still clicked on an object
			auto object = objRnd->getObject();
	
			// Add the object to the selection
			if(event->modifiers() == Qt::ControlModifier)
			{
				if(m_objectsSelection->isSelected(object))
					m_objectsSelection->remove(object);
				else
					m_objectsSelection->add(object);
			}
			else	// Moving the object (or selecting only this one if we release the mouse without moving)
			{
				if(!m_objectsSelection->isSelected(object))
					m_objectsSelection->selectOne(object);
				else
					m_objectsSelection->setLastSelectedObject(object);
				m_movingAction = Moving::Start;
				m_previousMousePos = zoomedMouse;
			}

			// Maybe do a custom action ?
			if(objRnd->mousePressEvent(event))
			{
				m_movingAction = Moving::Custom;
				m_capturedRenderer = objRnd;
			}
		}
		else
		{	// Clicked where there is nothing
			const auto modifiers = event->modifiers();
			if (modifiers == Qt::NoModifier)
			{
				m_movingAction = Moving::Selection; // Starting a rubber band to select in a zone
				m_objectsSelection->selectNone();
			}
			else if (modifiers & Qt::ShiftModifier)
			{
				if (modifiers & Qt::ControlModifier)
					m_movingAction = Moving::SelectionRemove;
				else
					m_movingAction = Moving::SelectionAdd;
			}
			else if (event->modifiers() == Qt::ControlModifier)
				m_movingAction = Moving::ZoomBox; // Starting a zoom box

			m_previousMousePos = m_currentMousePos = localPos;
			QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
		}
	}
	else if(event->button() == Qt::MidButton)
	{
		if(event->modifiers() == Qt::ControlModifier)
		{
			m_movingAction = Moving::Zoom;
			m_currentMousePos = convert(event->pos());
			m_previousMousePos = convert(event->pos());

			QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
		}
		else
		{
			m_movingAction = Moving::View;
			m_previousMousePos = convert(event->pos());

			QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
		}
	}
}

void GraphView::mouseMoveEvent(QMouseEvent* event)
{
	Point localPos = convert(event->localPos());

	if(m_movingAction == Moving::Start)
	{
		Point mousePos = viewport().toView(localPos);
		Point delta = mousePos - m_previousMousePos;
		if((delta * viewport().zoom()).norm() > 5)
		{
			m_movingAction = Moving::Object;
			if(m_useMagneticSnap && !m_selectedObjectsRenderers.empty())
			{
				auto objRnd = m_selectedObjectsRenderers.back();
				if(objRnd->acceptsMagneticSnap())
				{
					prepareSnapTargets(objRnd);
					auto possiblePosition = objRnd->getPosition() + delta;
					computeSnapDelta(objRnd, possiblePosition);
					delta = delta + m_snapDelta;
				}
			}
			QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));

			// Make sure docked objects are in the selection with their dock
			std::set<panda::PandaObject*> selectionSet;
			for(auto object : m_objectsSelection->get())
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

			m_moveObjectsMacro = m_pandaDocument->getUndoStack().beginMacro(tr("move objects").toStdString());

			if(!delta.isNull())
				m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(m_customSelection, delta));

			m_previousMousePos = mousePos;
		}
	}
	else if(m_movingAction == Moving::Object)
	{
		Point mousePos = viewport().toView(localPos);
		Point delta = mousePos - m_previousMousePos;
		if(m_useMagneticSnap && !m_selectedObjectsRenderers.empty())
		{
			Point oldSnapDelta = m_snapDelta;
			auto objRnd = m_selectedObjectsRenderers.back();
			auto possiblePosition = objRnd->getPosition() + delta - m_snapDelta;
			computeSnapDelta(objRnd, possiblePosition);
			delta = delta - oldSnapDelta + m_snapDelta;
		}

		if (!m_customSelection.empty() && !delta.isNull())
		{
			m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(m_customSelection, delta));
			m_recomputeLinks = true;
		}

		m_previousMousePos = mousePos;
	}
	else if(m_movingAction == Moving::View)
	{
		viewport().moveView((localPos - m_previousMousePos) / viewport().zoom());
		m_previousMousePos = localPos;
	}
	else if(m_movingAction == Moving::Zoom)
	{
		int y = localPos.y - m_previousMousePos.y;
		auto zoom = panda::helper::bound(0.1f, viewport().zoom() - y / 500.0f, 1.0f);
		viewport().setZoom(m_currentMousePos, zoom);
		m_previousMousePos = localPos;
	}
	else if(m_movingAction == Moving::Selection
			|| m_movingAction == Moving::SelectionAdd
			|| m_movingAction == Moving::SelectionRemove
			|| m_movingAction == Moving::ZoomBox)
	{
		m_currentMousePos = localPos;
		update();
	}
	else if(m_movingAction == Moving::Link)
	{
		m_currentMousePos = viewport().toView(localPos);

		auto dataRect = getDataAtPos(m_currentMousePos);
		if (dataRect.first && canLinkWith(dataRect.first))
			m_currentMousePos = dataRect.second.center();

		// Moving the view if the mouse is near the border of the widget
		moveViewIfMouseOnBorder();

		update();
	}
	else if(m_movingAction == Moving::Custom)
	{
		if(m_capturedRenderer)
			m_capturedRenderer->mouseMoveEvent(event);
	}

	if(m_movingAction == Moving::None || m_movingAction == Moving::Link)
	{
		Point zoomedMouse = viewport().toView(localPos);
		const auto dataRect = getDataAtPos(zoomedMouse);
		if(dataRect.first)
		{
			if(m_hoverData != dataRect.first)
			{
				m_hoverTimer->stop();
				if(m_highlightConnectedDatas)
				{
					m_highlightConnectedDatas = false;
					update();
				}
				m_hoverData = dataRect.first;
				m_hoverTimer->start(500);
			}

			auto label = DataLabelAddon::getDataLabel(m_hoverData);
					
			QString display;
			if(!label.empty())
				display = QString("<b>%1</b>").arg(QString::fromStdString(label));
			else
			{
				display = QString("%1\n%2")
					.arg(QString::fromStdString(m_hoverData->getName()))
					.arg(QString::fromStdString(m_hoverData->getDescription()));
			}

			QRect area = QRect(dataRect.second.left(), dataRect.second.top(), dataRect.second.width(), dataRect.second.height());
			QToolTip::showText(convert(toScreen(localPos)), display, this, area);
			if(!m_hoverData->getHelp().empty())
				emit showStatusBarMessage(QString::fromStdString(m_hoverData->getHelp()));
		}
		else
		{
			m_hoverData = nullptr;
			if(m_highlightConnectedDatas)
			{
				m_highlightConnectedDatas = false;
				update();
			}
			if(m_hoverTimer->isActive())
				m_hoverTimer->stop();

			if(m_movingAction == Moving::None)
			{
				// Look for link tags
				for(auto& tag : m_linkTags)
				{
					auto dataPair = tag->getDataAtPoint(zoomedMouse);
					auto data = dataPair.first;
					bool hovering = data != nullptr;
					if(hovering != tag->isHovering())
					{
						tag->setHovering(hovering);
						update();
					}

					if (!data)
						continue;

					auto label = DataLabelAddon::getDataLabel(tag->getInputData());
					
					QString display;
					if(!label.empty())
						display = QString("<b>%1</b>").arg(QString::fromStdString(label));
					else if (data && data->isInput())
					{
						auto parent = data->getParent();
						display = QString("%1\n%2")
							.arg(QString::fromStdString(parent->getOwner()->getName()))
							.arg(QString::fromStdString(parent->getName()));
					}

					if (!display.isEmpty())
					{
						auto tagRect = dataPair.second;
						auto rect = QRect(tagRect.left(), tagRect.top(), tagRect.width(), tagRect.height());
						QToolTip::showText(convert(toScreen(localPos)), display, this, rect);
					}
				}
			}
		}
	}
}

void GraphView::mouseReleaseEvent(QMouseEvent* event)
{
	if(m_movingAction == Moving::Start)
	{
		panda::PandaObject* object = m_objectsSelection->lastSelectedObject();
		if(object)
			m_objectsSelection->selectOne(object);
	}
	else if(m_movingAction == Moving::Object)
	{
		std::map<panda::PandaObject*, Point> positions;
		for(const auto objRnd : m_selectedObjectsRenderers)
			positions[objRnd->getObject()] = objRnd->getPosition();

		for(const auto objRnd : m_selectedObjectsRenderers)
		{
			auto object = objRnd->getObject();
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			if(dockable && !m_objectsSelection->isSelected(dockable->getParentDock()))
			{
				Point delta = positions[object] - objRnd->getPosition();
				m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(dockable, delta));

				Rect dockableArea = objRnd->getSelectionArea();
				panda::DockObject* defaultDock = dockable->getDefaultDock();
				panda::DockObject* newDock = defaultDock;
				int newIndex = -1;
				for(const auto objRnd2 : objectRenderers().getOrdered())
				{
					panda::DockObject* dock = dynamic_cast<panda::DockObject*>(objRnd2->getObject());
					if(dock)
					{
						if(dockableArea.intersects(objRnd2->getSelectionArea()) && dock->accepts(dockable))
						{
							newIndex = dynamic_cast<object::DockObjectRenderer*>(objRnd2)->getDockableIndex(dockableArea);
							newDock = dock;
							break;
						}
					}
				}

				panda::DockObject* prevDock = dockable->getParentDock();
				if(newDock != prevDock) // Changing dock
				{
					if(prevDock)
						m_pandaDocument->getUndoStack().push(std::make_shared<panda::DetachDockableCommand>(prevDock, dockable));
					if(newDock)
					{
						m_pandaDocument->getUndoStack().push(std::make_shared<panda::AttachDockableCommand>(newDock, dockable, newIndex));
						m_pandaDocument->onChangedDock(dockable);
					}
				}
				else if(prevDock != defaultDock) // (maybe) Changing place in the dock
				{
					int prevIndex = prevDock->getIndexOfDockable(dockable);
					if(prevIndex != newIndex)
					{
						if(newIndex > prevIndex)
							--newIndex;

						m_pandaDocument->getUndoStack().push(std::make_shared<panda::ReorderDockableCommand>(prevDock, dockable, newIndex));
					}
					modifiedObject(prevDock); // Always update
				}
				else if(defaultDock) // (maybe) Changing place in the default dock
					sortDockable(dockable, defaultDock);
			}
		}

		m_moveObjectsMacro.reset();

		viewport().updateObjectsRect();
	}
	else if(m_movingAction == Moving::View)
	{
		emit viewportModified();
	}
	else if(m_movingAction == Moving::Zoom)
	{
		viewport().updateViewRect();
	}
	else if(m_movingAction == Moving::ZoomBox)
	{
		viewport().setViewport({ m_previousMousePos, m_currentMousePos });
	}
	else if(m_movingAction == Moving::Selection
			|| m_movingAction == Moving::SelectionAdd
			|| m_movingAction == Moving::SelectionRemove)
	{
		const auto zoom = viewport().zoom();
		bool remove = m_movingAction == Moving::SelectionRemove;
		ObjectsSelection::Objects selection = m_objectsSelection->get();
		Rect selectionRect = Rect(m_previousMousePos / zoom, m_currentMousePos / zoom)
			.translated(viewport().viewDelta())
			.canonicalized();
		for(const auto objRnd : objectRenderers().getOrdered())
		{
			Rect objectArea = objRnd->getSelectionArea();
			if (selectionRect.intersects(objectArea))
			{
				auto object = objRnd->getObject();
				if (remove)
					panda::helper::removeOne(selection, object);
				else if(!panda::helper::contains(selection, object))
					selection.push_back(objRnd->getObject());
			}
		}

		m_objectsSelection->set(selection);
		update();
	}
	else if(m_movingAction == Moving::Link)
	{
		const auto dataRect = getDataAtPos(m_currentMousePos);
		auto data = dataRect.first;
		if(data && canLinkWith(data))
		{
			if (createLink(m_clickedData, data))
				m_recomputeTags = true;
		}
		m_clickedData = nullptr;
		update();
	}
	else if(m_movingAction == Moving::Custom)
	{
		if(m_capturedRenderer)
		{
			m_capturedRenderer->mouseReleaseEvent(event);
			m_capturedRenderer = nullptr;
			viewport().updateObjectsRect();
		}
	}

	QApplication::restoreOverrideCursor();
	m_movingAction = Moving::None;
}

void GraphView::wheelEvent(QWheelEvent* event)
{
	if(m_movingAction != Moving::None)
	{
		event->ignore();
		return;
	}

	m_wheelTicks += event->angleDelta().y();
	int ticks = m_wheelTicks / 40; // Steps of 5 degrees
	m_wheelTicks -= ticks * 40;
	const auto zoomLevel = viewport().zoomLevel();
	int newZoom = panda::helper::bound(0, zoomLevel - ticks, 90);
	viewport().setZoomLevel(convert(event->pos()), newZoom);
}

void GraphView::keyPressEvent(QKeyEvent* event)
{
	switch(event->key())
	{
	case Qt::Key_Space:
	{
		if(!m_pandaDocument->animationIsPlaying())
		{
			QuickCreateDialog dlg(m_pandaDocument, this);
			dlg.exec();
		}
		break;
	}
	case Qt::Key_Left:
		if(event->modifiers() & Qt::ControlModifier)
			viewport().moveView(Point(100, 0));
		break;
	case Qt::Key_Right:
		if(event->modifiers() & Qt::ControlModifier)
			viewport().moveView(Point(-100, 0));
		break;
	case Qt::Key_Up:
		if(event->modifiers() & Qt::ControlModifier)
			viewport().moveView(Point(0, 100));
		break;
	case Qt::Key_Down:
		if(event->modifiers() & Qt::ControlModifier)
			viewport().moveView(Point(0, -100));
		break;
	case Qt::Key_Plus:
		if(event->modifiers() & Qt::ControlModifier)
			viewport().zoomIn();
		break;
	case Qt::Key_Minus:
		if(event->modifiers() & Qt::ControlModifier)
			viewport().zoomOut();
		break;
	default:
		QWidget::keyPressEvent(event);
	}
}

void GraphView::contextMenuEvent(QContextMenuEvent* event)
{
	m_contextMenuData = nullptr;
	
	Point pos = viewport().toView(convert(event->pos()));
	QMenu menu(this);
	int flags = getContextMenuFlags(pos);
	
	if(m_hoverTimer->isActive())
		m_hoverTimer->stop();

	const auto gPos = event->globalPos();
	const auto posI = panda::graphics::PointInt(gPos.x(), gPos.y());
	m_pandaDocument->getGUI().contextMenu(posI, flags);
}

int GraphView::getContextMenuFlags(const panda::types::Point& pos)
{
	namespace gm = panda::gui::menu;
	int flags = gm::Selection; // Let MainWindow fill the menu based on the current selection
	const auto objRnd = objectRenderers().getAtPos(pos);
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
	for (const auto& linkTag : m_linkTags)
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

void GraphView::addedObject(panda::PandaObject* object)
{
	// Creating a Renderer depending on the class of the object been added
	auto objRnd = objectRenderers().get(object);
	if (!objRnd)
	{
		auto objRndPtr = object::ObjectRendererFactory::getInstance()->createRenderer(this, object);
		objectRenderers().set(object, objRndPtr);
		objRnd = objRndPtr.get();
	}

	objRnd->setDirty();
	update();
}

void GraphView::removeObject(panda::PandaObject* object)
{
	objectRenderers().remove(object);
	m_capturedRenderer = nullptr;
	m_movingAction = Moving::None;
	m_linkTags.clear();
	m_linkTagsMap.clear();
	m_recomputeTags = true;
	m_recomputeLinks = true;
	m_highlightConnectedDatas = false;

	viewport().updateObjectsRect();
}

void GraphView::modifiedObject(panda::PandaObject* object)
{
	m_recomputeLinks = true;
	auto objRnd = objectRenderers().get(object);
	if(objRnd)	// Can be called before the object is fully created
	{
		objRnd->setDirty();

		panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object);
		if (dock)
		{
			auto dobjRnd = dynamic_cast<object::DockObjectRenderer*>(objRnd);
			if (dobjRnd)
				dobjRnd->placeDockableObjects();
		}

		update();
	}
}

int GraphView::getAvailableLinkTagIndex()
{
	int nb = m_linkTags.size();
	std::vector<bool> indices(nb, true);

	for(const auto& linkTag : m_linkTags)
	{
		int id = linkTag->index();
		if(id < nb)
			indices[id] = false;
	}

	for(int i=0; i<nb; ++i)
	{
		if(indices[i])
			return i;
	}

	return nb;
}

void GraphView::addLinkTag(panda::BaseData* input, panda::BaseData* output)
{
	if(m_linkTagsMap.count(input))
		m_linkTagsMap[input]->addOutput(output);
	else
	{
		auto tag = std::make_shared<LinkTag>(this, input, output, getAvailableLinkTagIndex());
		m_linkTags.push_back(tag);
		m_linkTagsMap[input] = tag.get();
	}
}

void GraphView::removeLinkTag(panda::BaseData* input, panda::BaseData* output)
{
	if(m_linkTagsMap.count(input))
	{
		LinkTag* tag = m_linkTagsMap[input];
		tag->removeOutput(output);
		if (tag->isEmpty())
		{
			m_linkTagsMap.erase(input);
			panda::helper::removeIf(m_linkTags, [tag](const auto& ptr) {
				return ptr.get() == tag;
			});
		}
	}
}

void GraphView::updateLinkTags()
{
	m_recomputeTags = false;
	// Testing all links and adding new tags
	for(auto& object : m_objectsList.get())
	{
		for(auto& data : object->getInputDatas())
		{
			panda::BaseData* parentData = data->getParent();
			if(parentData)
			{
				auto ownerObjRnd = objectRenderers().get(data->getOwner());
				auto parentObjRnd = objectRenderers().get(parentData->getOwner());
				if (ownerObjRnd && parentObjRnd)
				{
					Rect ownerRect, parentRect;
					ownerObjRnd->getDataRect(data, ownerRect);
					parentObjRnd->getDataRect(parentData, parentRect);
					float ox = ownerRect.center().x;
					float ix = parentRect.center().x;
					if (LinkTag::needLinkTag(ix, ox, this))
						addLinkTag(parentData, data);
				}
			}
		}
	}

	// Updating tags
	for (auto& linkTag : m_linkTags)
		linkTag->update();

	// Removing empty ones from the map
	for (auto it = m_linkTagsMap.begin(); it != m_linkTagsMap.end();)
	{
		if (it->second->isEmpty())
			it = m_linkTagsMap.erase(it);
		else
			++it;
	}

	// And removing them from the list (freeing them in the process)
	auto last = std::remove_if(m_linkTags.begin(), m_linkTags.end(), [](const auto& tag) {
		return tag->isEmpty();
	});
	m_linkTags.erase(last, m_linkTags.end());

	// Updating the connections list
	m_linkTagsDatas.clear();
	for (const auto& linkTag : m_linkTags)
	{
		const auto input = linkTag->getInputData();
		for (const auto output : linkTag->getOutputDatas())
			m_linkTagsDatas.emplace(input, output);
	}
}

void GraphView::removeLink()
{
	if(m_contextMenuData && m_contextMenuData->isInput() && m_contextMenuData->getParent())
	{
		removeLinkTag(m_contextMenuData->getParent(), m_contextMenuData);
		changeLink(m_contextMenuData, nullptr);
		m_contextMenuData = nullptr;
		update();
	}
}

void GraphView::hoverDataInfo()
{
	if(m_hoverData)
	{
		m_highlightConnectedDatas = true;
		m_recomputeConnected = true;
		update();
	}
}

void GraphView::updateLinks()
{
	m_recomputeLinks = false;
	m_linksDrawList.clear();

	auto col = m_drawColors.penColor;

	for(const auto objRnd : objectRenderers().getOrdered())
	{
		for (const auto& toDataRect : objRnd->getDataRects())
		{
			panda::BaseData* data = toDataRect.first;
			panda::BaseData* parent = data->getParent();
			if (parent && !data->isOutput())
			{
				Rect fromDataRect;
				auto objRnd = objectRenderers().get(parent->getOwner());
				if (objRnd && objRnd->getDataRect(parent, fromDataRect) 
					&& !hasLinkTag(parent, data)) // We don't draw the link if there is a LinkTag
				{
					auto d1 = fromDataRect.center(), d2 = toDataRect.second.center();
					Point w = { (d2.x - d1.x) / 2, 0 };
					m_linksDrawList.addBezierCurve(d1, d1 + w, d2 - w, d2, col, 1);
				}
			}
		}
	}
}

void GraphView::updateConnectedDatas()
{
	m_recomputeConnected = false;
	m_connectedDrawList.clear();
	if (!m_highlightConnectedDatas)
		return;

	auto connected = getConnectedDatas(m_hoverData);
	const auto& rects = connected.first;
	const auto& links = connected.second;

	if(links.empty())
		return;

	// Now draw everything
	for (const auto& rect : rects)
	{
		m_connectedDrawList.addRectFilled(rect, m_drawColors.highlightColor);
		m_connectedDrawList.addRect(rect, m_drawColors.penColor, 1.f);
	}

	for(const auto& link : links)
	{
		float w = (link.second.x - link.first.x) / 2;
		auto p1 = link.first, p2 = link.second, d = Point(w, 0);
		m_connectedDrawList.addBezierCurve(p1, p1 + d, p2 - d, p2, m_drawColors.highlightColor, 3);
	}
}

std::pair<GraphView::Rects, GraphView::PointsPairs> GraphView::getConnectedDatas(panda::BaseData* srcData)
{
	GraphView::Rects rects;
	GraphView::PointsPairs links;

	Rect sourceRect;
	if(getDataRect(srcData, sourceRect))
		rects.push_back(sourceRect);
	else
		return{ rects, links };

	// Get outputs
	if(srcData->isOutput())
	{
		for(const auto node : srcData->getOutputs())
		{
			panda::BaseData* data = dynamic_cast<panda::BaseData*>(node);
			if(data)
			{
				Rect rect;
				if (getDataRect(data, rect))
				{
					rects.push_back(rect);
					links.emplace_back(rect.center(), sourceRect.center());
				}
			}
		}
	}
	// Or the one input
	else if(srcData->isInput())
	{
		panda::BaseData* data = srcData->getParent();
		if(data)
		{
			Rect rect;
			if(getDataRect(data, rect))
			{
				rects.push_back(rect);
				links.emplace_back(sourceRect.center(), rect.center());
			}
		}
	}

	return{ rects, links };
}

void GraphView::prepareSnapTargets(object::ObjectRenderer* selectedRenderer)
{
	m_snapTargetsY.clear();

	float y = selectedRenderer->getPosition().y;
	// For y, try to make the data links horizontal
	// First for inputs
	for(auto input : selectedRenderer->getObject()->getInputDatas())
	{
		Rect dataRect;
		if(selectedRenderer->getDataRect(input, dataRect))
		{
			auto dataHeight = dataRect.center().y - y;
			for(auto input2 : input->getInputs())
			{
				auto data2 = dynamic_cast<panda::BaseData*>(input2);
				if(data2 && data2->getOwner())
				{
					auto owner = data2->getOwner();
					auto objRnd = objectRenderers().get(owner);
					if(objRnd)
					{
						if(objRnd->getDataRect(data2, dataRect))
							m_snapTargetsY.insert(dataRect.center().y - dataHeight);
					}
				}
			}
		}
	}

	// Then for outputs
	for(auto output : selectedRenderer->getObject()->getOutputDatas())
	{
		Rect dataRect;
		if(selectedRenderer->getDataRect(output, dataRect))
		{
			auto dataHeight = dataRect.center().y - y;
			for(auto output2 : output->getOutputs())
			{
				auto data2 = dynamic_cast<panda::BaseData*>(output2);
				if(data2 && data2->getOwner())
				{
					auto owner = data2->getOwner();
					auto objRnd = objectRenderers().get(owner);
					if (objRnd)
					{
						if(objRnd->getDataRect(data2, dataRect))
							m_snapTargetsY.insert(dataRect.center().y - dataHeight);
					}
				}
			}
		}
	}
}

void GraphView::computeSnapDelta(object::ObjectRenderer* selectedRenderer, Point position)
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
	auto selectedHeight = selectedRenderer->getObjectSize().y;
	auto displayRect = viewport().displayRect();
	auto m1 = std::numeric_limits<float>::lowest(), m2 = std::numeric_limits<float>::max();
	Point abovePos(m1, m1), belowPos(m2, m2);
	float aboveDist{ m2 }, belowDist{ m2 };
	bool hasInsideObject = false;
	std::set<float> snapTargetsX;
	for (const auto objRnd : objectRenderers().getOrdered())
	{
		if (objRnd == selectedRenderer || !objRnd->acceptsMagneticSnap())
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
					abovePos = Point(pos.x , pos.y + area.height());
				}
			}
			else if(pos.y > position.y + selectedHeight)
			{
				// Distance from the top left corner of this one and the bottom left of the selected
				auto dist = Point(pos.x - position.x, pos.y - (position.y + selectedHeight)).norm();
				if (dist < belowDist)
				{
					belowDist = dist;
					belowPos = Point(pos.x , pos.y - selectedHeight);
				}
			}
			else if(qAbs(pos.x - position.x) < filterDist) // The selected one intersects the y axis of this one, and is close enough on the x axis
			{
				snapTargetsX.insert(pos.x);
				hasInsideObject = true;
			}
		}
	}
	
	if (hasInsideObject)
	{
		// Only take the other ones if their are close
		if(qAbs(abovePos.y - position.y) < filterDist)
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
	if(minIter != snapTargetsX.end())
	{
		float x = *minIter;
		if(qAbs(x - position.x) < snapMaxDist)
			m_snapDelta.x = x - position.x;
	}

	minIter = std::min_element(m_snapTargetsY.begin(), m_snapTargetsY.end(), comparator(position.y));
	if(minIter != m_snapTargetsY.end())
	{
		float y = *minIter;
		if(qAbs(y - position.y) < snapMaxDist)
			m_snapDelta.y = y - position.y;
	}
}

void GraphView::moveObjects(std::vector<panda::PandaObject*> objects, Point delta)
{
	if(delta.isNull())
		return;

	for(auto object : objects)
	{
		auto objRnd = objectRenderers().get(object);
		if(objRnd)
			objRnd->move(delta);
	}

	objectsMoved();
}

void GraphView::objectsMoved()
{
	m_objectsMoved = true;
	update();
}

bool GraphView::createLink(panda::BaseData* data1, panda::BaseData* data2)
{
	panda::BaseData *target = nullptr, *parent = nullptr;
	if (data1->isInput() && data2->isOutput())
	{
		changeLink(data1, data2);
		return true;
	}
	else if (data2->isInput() && data1->isOutput())
	{
		changeLink(data2, data1);
		return true;
	}
	else
		return false;
}

void GraphView::changeLink(panda::BaseData* target, panda::BaseData* parent)
{
	auto macro = m_pandaDocument->getUndoStack().beginMacro(tr("change link").toStdString());
	m_pandaDocument->getUndoStack().push(std::make_shared<panda::LinkDatasCommand>(target, parent));
}

void GraphView::sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock)
{
	int prevIndex = defaultDock->getIndexOfDockable(dockable);
	auto dockables = defaultDock->getDockedObjects();

	std::sort(dockables.begin(), dockables.end(), [this](panda::DockableObject* lhs, panda::DockableObject* rhs){
		auto lpos = objectRenderers().get(lhs)->getPosition();
		auto rpos = objectRenderers().get(rhs)->getPosition();
		if(lpos.y == rpos.y)
			return lpos.x > rpos.x;
		return lpos.y < rpos.y;
	});

	auto iter = std::find(dockables.begin(), dockables.end(), dockable);
	if(iter != dockables.end())
	{
		int newIndex = iter - dockables.begin();
		if(newIndex == prevIndex)
			return;

		m_pandaDocument->getUndoStack().push(std::make_shared<panda::ReorderDockableCommand>(defaultDock, dockable, newIndex));
	}
}

void GraphView::sortDockablesInDock(panda::DockObject* dock)
{
	for(const auto dockable : dock->getDockedObjects())
	{
		auto defaultDock = dockable->getDefaultDock();
		auto parentDock = dockable->getParentDock();
		if(defaultDock && parentDock == defaultDock)
			sortDockable(dockable, defaultDock);
	}
}

void GraphView::sortAllDockables()
{
	for(const auto& object : m_objectsList.get())
	{
		const auto dockable = dynamic_cast<panda::DockableObject*>(object.get());
		if(!dockable)
			continue;
		auto defaultDock = dockable->getDefaultDock();
		auto parentDock = dockable->getParentDock();
		if(defaultDock && parentDock == defaultDock)
			sortDockable(dockable, defaultDock);
	}
}

void GraphView::startLoading()
{
	m_isLoading = true;
}

void GraphView::loadingFinished()
{
	m_isLoading = false;
	sortAllDockables();
	updateDirtyRenderers();
}

void GraphView::changedDock(panda::DockableObject* dockable)
{
	auto defaultDock = dockable->getDefaultDock();
	auto parentDock = dockable->getParentDock();
	if(defaultDock && parentDock == defaultDock)
		sortDockable(dockable, defaultDock);
}

QSize GraphView::viewSize()
{
	return QSize(m_viewRect.width(), m_viewRect.height());
}

QPoint GraphView::viewPosition()
{
	auto delta = viewport().viewDelta() * viewport().zoom();
	return QPoint(m_viewRect.left() - delta.x, m_viewRect.top() - delta.y);
}

void GraphView::scrollView(QPoint position)
{
	Point delta = convert(position) - m_viewRect.topLeft() + viewport().viewDelta() * viewport().zoom();
	viewport().moveView(delta);
}

void GraphView::showChooseWidgetDialog()
{
	if(m_contextMenuData)
		ChooseWidgetDialog(m_contextMenuData, this).exec();
	else
	{
		auto obj = m_objectsSelection->lastSelectedObject();
		if(obj && obj->getClass()->getClassName() == "GeneratorUser" && obj->getClass()->getNamespaceName() == "panda")
		{
			auto data = obj->getData("input");
			if(data)
				ChooseWidgetDialog(data, this).exec();
		}
	}
}

void GraphView::focusOutEvent(QFocusEvent*)
{
	emit lostFocus(this);
}

void GraphView::selectionChanged()
{
	m_selectedObjectsRenderers.clear();

	for (auto object : m_objectsSelection->get())
	{
		auto objRnd = objectRenderers().get(object);
		if(objRnd)
			m_selectedObjectsRenderers.push_back(objRnd);
	}

	update();
}

bool GraphView::isCompatible(const panda::BaseData* data1, const panda::BaseData* data2)
{
	if(data1->getOwner() == data2->getOwner())
		return false;

	if(data1->isInput())
	{
		if(!data2->isOutput())
			return false;
		return data1->validParent(data2);
	}
	else if(data2->isInput())
	{
		if(!data1->isOutput())
			return false;
		return data2->validParent(data1);
	}

	return false;
}

void GraphView::computeCompatibleDatas(panda::BaseData* data)
{
	std::vector<panda::BaseData*> forbiddenList;
	if (data->isInput())
		forbiddenList = panda::graph::extractDatas(panda::graph::computeConnectedOutputNodes(data, false));
	else if(data->isOutput())
		forbiddenList = panda::graph::extractDatas(panda::graph::computeConnectedInputNodes(data, false));
	std::sort(forbiddenList.begin(), forbiddenList.end());

	m_possibleLinks.clear();
	for (const auto& object : m_objectsList.get())
	{
		for (const auto linkData : object->getDatas())
		{
			if (isCompatible(data, linkData) 
				&& !std::binary_search(forbiddenList.begin(), forbiddenList.end(), linkData))
				m_possibleLinks.insert(linkData);
		}
	}
}

void GraphView::updateDirtyRenderers()
{
	bool updated = false;

	// Bugfix: update the dock objects last
	const auto& orderedObjectRenderers = objectRenderers().getOrdered();
	for (auto objRnd : orderedObjectRenderers)
		if(!dynamic_cast<panda::DockObject*>(objRnd->getObject()))
			updated |= objRnd->updateIfDirty();

	for (auto objRnd : orderedObjectRenderers)
		if(dynamic_cast<panda::DockObject*>(objRnd->getObject()))
			updated |= objRnd->updateIfDirty();

	if (!updated)
		return;

	m_linkTags.clear();
	m_linkTagsMap.clear();
	m_recomputeTags = true;
	m_highlightConnectedDatas = false;
	m_hoverData = nullptr;
	m_hoverTimer->stop();
	m_recomputeLinks = true;
	m_recomputeConnected = true;
	
	viewport().updateObjectsRect();
}

void GraphView::objectsReordered()
{
	objectRenderers().reorder(m_objectsList);
}

panda::types::Point GraphView::getNewObjectPosition()
{
	return contentsArea().center() + viewport().viewDelta();
}

void GraphView::setDataLabel()
{
	if (!m_contextLinkTag && !m_contextMenuData)
		return;

	auto data = m_contextLinkTag ? m_contextLinkTag->getInputData() : m_contextMenuData;
	auto label = DataLabelAddon::getDataLabel(data);

	bool ok = false;
	label = QInputDialog::getMultiLineText(this, tr("Data label"), tr("Label:"), QString::fromStdString(label), &ok).toStdString();
	if (!ok)
		return;

	DataLabelAddon::setDataLabel(data, label);

	emit modified();
}

void GraphView::moveViewIfMouseOnBorder()
{
	auto localPos = viewport().fromView(m_currentMousePos);
	auto contents = contentsRect();
	auto area = contentsArea();
	const float maxDist = 50;
	area.adjust(maxDist, maxDist, -maxDist, -maxDist);
	if (!area.contains(localPos))
	{
		float dx = 0, dy = 0;
		if (localPos.x < area.left())
			dx = area.left() - localPos.x;
		else if (localPos.x > area.right())
			dx = area.right() - localPos.x;

		if (localPos.y < area.top())
			dy = area.top() - localPos.y;
		else if (localPos.y > area.bottom())
			dy = area.bottom() - localPos.y;

		auto now = currentTime();
		if (now > m_previousTime)
		{
			float dt = (now - m_previousTime) / 1000000.f;
			const float speed = 10.f / viewport().zoom();
			m_previousTime = now;
			Point delta = speed * dt * Point(dx, dy);
			viewport().moveView(delta);
		}
	}

	update();
}

void GraphView::moveObjectToBack()
{
	assert(m_contextMenuObject);
	m_objectsList.reinsertObject(m_contextMenuObject, 0); // Front of the list = others are drawn on top
}

void GraphView::moveObjectToFront()
{
	assert(m_contextMenuObject);
	m_objectsList.reinsertObject(m_contextMenuObject, -1); // Back of the list = drawn on top of the others
}

void GraphView::copy()
{
	const auto& selection = m_objectsSelection->get();
	if (selection.empty())
		return;

	QApplication::clipboard()->setText(QString::fromStdString(panda::serialization::writeTextDocument(m_pandaDocument, selection)));
}

void GraphView::cut()
{
	copy();
	del();
}

void GraphView::paste()
{
	const QMimeData* mimeData = QApplication::clipboard()->mimeData();
	if (!mimeData->hasText())
		return;
	
	auto result = panda::serialization::readTextDocument(m_pandaDocument, objectsList(), mimeData->text().toStdString());
	if (!result.first || result.second.empty())
		return;

	m_objectsSelection->set(result.second);
	viewport().moveSelectedToCenter();

	m_pandaDocument->getUndoStack().push(std::make_shared<AddObjectCommand>(m_pandaDocument, m_objectsList, result.second));
}

void GraphView::del()
{
	const auto& selection = m_objectsSelection->get();
	if (selection.empty())
		return;

	auto macro = m_pandaDocument->getUndoStack().beginMacro(tr("delete objects").toStdString());	
	m_pandaDocument->getUndoStack().push(std::make_shared<RemoveObjectCommand>(m_pandaDocument, m_objectsList, selection));
}

void GraphView::executeNextRefresh(std::function<void()> func)
{
	m_functionsToExecuteNextRefresh.push_back(func);
}

panda::types::Rect GraphView::contentsArea() const
{
	return convert(contentsRect());
}

panda::types::Point GraphView::toScreen(const panda::types::Point& pos) const
{
	return convert(mapToGlobal(convert(pos)));
}

void GraphView::emitViewportModified()
{
	emit viewportModified();
}

} // namespace graphview
