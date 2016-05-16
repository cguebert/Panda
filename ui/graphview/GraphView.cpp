#include <QtWidgets>
#include <chrono>
#include <functional>
#include <limits>

#include <ui/command/AddObjectCommand.h>
#include <ui/command/MoveObjectCommand.h>
#include <ui/command/RemoveObjectCommand.h>
#include <ui/dialog/ChooseWidgetDialog.h>
#include <ui/dialog/QuickCreateDialog.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/drawstruct/DockableDrawStruct.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/LinkTag.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/ViewRenderer.h>
#include <ui/graphview/graphics/DrawList.h>

#include <panda/PandaDocument.h>
#include <panda/helper/algorithm.h>
#include <panda/types/DataTraits.h>
#include <panda/command/DockableCommand.h>
#include <panda/command/LinkDatasCommand.h>
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
	bool isCompatible(const panda::BaseData* data1, const panda::BaseData* data2)
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

	inline panda::types::Point convert(const QPointF& pt)
	{ return panda::types::Point(static_cast<float>(pt.x()), static_cast<float>(pt.y())); }

	long long currentTime()
	{
		using namespace std::chrono;
		auto now = time_point_cast<microseconds>(high_resolution_clock::now());
		return now.time_since_epoch().count();
	}
}

GraphView::GraphView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent)
	: QOpenGLWidget(parent)
	, m_pandaDocument(doc)
	, m_objectsList(objectsList)
	, m_hoverTimer(new QTimer(this))
	, m_objectsSelection(std::make_unique<ObjectsSelection>(objectsList))
	, m_viewRenderer(std::make_unique<ViewRenderer>())
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

	connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverDataInfo()));

	m_hoverTimer->setSingleShot(true);

	setMouseTracking(true);

	const auto& pal = palette();
	m_drawColors.penColor = DrawList::convert(pal.text().color());
	m_drawColors.midLightColor = DrawList::convert(pal.midlight().color());
	m_drawColors.lightColor = DrawList::convert(pal.light().color());
	m_drawColors.highlightColor = DrawList::convert(pal.highlight().color());

	// Create the draw structs for the objects already present
	for (const auto& object : m_objectsList.get())
		addedObject(object.get());
}

GraphView::~GraphView() = default;

void GraphView::resetView()
{
	m_viewDelta = Point();
	m_zoomLevel = 0;
	m_zoomFactor = 1.0;
	m_objectDrawStructs.clear();
	m_orderedObjectDrawStructs.clear();
	m_linkTags.clear();
	m_linkTagsMap.clear();
	m_movingAction = Moving::None;
	m_clickedData = nullptr;
	m_hoverData = nullptr;
	m_contextMenuData = nullptr;
	m_capturedDrawStruct = nullptr;
	m_recomputeTags = false;
	m_hoverTimer->stop();
	m_highlightConnectedDatas = false;
	m_useMagneticSnap = true;
	m_isLoading = false;
	m_objectsRect = Rect();
	m_viewRect = Rect();
	m_selectedObjectsDrawStructs.clear();
	m_possibleLinks.clear();
	m_dirtyDrawStructs.clear();
	m_dirtyDrawStructsSet.clear();
	m_linksDrawList.clear();
	m_connectedDrawList.clear();

	emit viewModified();
}

QSize GraphView::minimumSizeHint() const
{
	return QSize(300, 200);
}

QSize GraphView::sizeHint() const
{
	return QSize(600, 400);
}

ObjectDrawStruct* GraphView::getObjectDrawStructAtPos(const Point& pt)
{
	int nb = m_orderedObjectDrawStructs.size();
	for (int i = nb - 1; i >= 0; --i)
	{
		auto ods = m_orderedObjectDrawStructs[i];
		if(ods->contains(pt))
			return ods;
	}
	return nullptr;
}

std::pair<panda::BaseData*, Rect> GraphView::getDataAtPos(const panda::types::Point& pt)
{
	const auto ods = getObjectDrawStructAtPos(pt);
	if (ods)
	{
		panda::BaseData* data = ods->getDataAtPos(pt);
		Rect dataRect;
		if (ods->getDataRect(data, dataRect))
			return{ data, dataRect };
	}

	return{ nullptr, Rect() };
}

GraphView::ObjectDrawStructPtr GraphView::getSharedObjectDrawStruct(panda::PandaObject* object)
{
	return panda::helper::valueOrDefault(m_objectDrawStructs, object);
}

ObjectDrawStruct* GraphView::getObjectDrawStruct(panda::PandaObject* object)
{
	return panda::helper::valueOrDefault(m_objectDrawStructs, object).get();
}

std::vector<ObjectDrawStruct*> GraphView::getObjectDrawStructs(const std::vector<panda::PandaObject*>& objects)
{
	std::vector<ObjectDrawStruct*> odsList;
	for (auto object : objects)
	{
		auto ods = getObjectDrawStruct(object);
		if(ods)
			odsList.push_back(ods);
	}

	return odsList;
}

void GraphView::setObjectDrawStruct(panda::PandaObject* object, const ObjectDrawStructPtr& drawStruct)
{
	m_objectDrawStructs[object] = drawStruct;
	m_orderedObjectDrawStructs.push_back(drawStruct.get());
}

void GraphView::initializeGL()
{
	m_viewRenderer->initialize();
	m_linksDrawList = DrawList();
	m_connectedDrawList = DrawList();
}

void GraphView::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	m_viewRenderer->resize(w, h);
	update();
}

void GraphView::paintGL()
{
	// Moving the view when creating a link if the mouse is near the border of the widget
	if(m_movingAction == Moving::Link)
		moveViewIfMouseOnBorder();

	updateDirtyDrawStructs();

	if(m_recomputeTags)			updateLinkTags();
	if(m_recomputeLinks)		updateLinks();
	if(m_recomputeConnected)	updateConnectedDatas();

	Rect viewRect(m_viewDelta, width() / m_zoomFactor, height() / m_zoomFactor);
	m_viewRenderer->setView(viewRect);
	m_viewRenderer->newFrame();
	DrawList drawList;

	auto col = palette().background().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Give a possibility to draw behind normal objects
	for (auto& ods : m_orderedObjectDrawStructs)
	{
		if (ods->getVisualArea().intersects(viewRect))
			ods->drawBackground(drawList, m_drawColors);
	}

	// Draw links
	drawList.merge(m_linksDrawList);

	// Draw the objects
	for (auto& ods : m_orderedObjectDrawStructs)
	{
		if (ods->getVisualArea().intersects(viewRect))
			ods->draw(drawList, m_drawColors);
	}

	// Give a possibility to draw in front of normal objects
	for (auto& ods : m_orderedObjectDrawStructs)
	{
		if (ods->getVisualArea().intersects(viewRect))
			ods->drawForeground(drawList, m_drawColors);
	}

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for (auto& ods : m_selectedObjectsDrawStructs)
	{
		if (ods->getVisualArea().intersects(viewRect))
			ods->draw(drawList, m_drawColors, true);
	}

	// Draw links tags
	for (auto& tag : m_linkTags)
		tag->draw(drawList, m_drawColors);

	// Selection rubber band
	if (m_movingAction == Moving::Selection)
	{
		auto r = Rect(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor).translated(m_viewDelta).canonicalized();
		auto highlight = m_drawColors.highlightColor;
		highlight = (highlight & 0x00FFFFFF) | 0x40000000;
		drawList.addRectFilled(r, highlight);
		drawList.addRect(r, m_drawColors.penColor, 0.75f / m_zoomFactor);
	}

	// Zoom box
	if (m_movingAction == Moving::ZoomBox)
	{
		auto r = Rect(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor).translated(m_viewDelta).canonicalized();
		drawList.addRect(r, m_drawColors.penColor, 0.75f / m_zoomFactor);
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
void GraphView::paintLogDebug(DrawList& list, DrawColors& colors)
{
	UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
	if(logDlg && logDlg->isVisible())
	{
		const auto& states = logDlg->getNodeStates();
		for(const auto ods : m_orderedObjectDrawStructs)
		{
			const auto object = ods->getObject();
			unsigned int fillCol = panda::helper::valueOrDefault(states, object, nullptr) ? 0x200000FF : 0x2000FF00;
	
			auto area = ods->getVisualArea();
			list.addRectFilled(area, fillCol);

			for(panda::BaseData* data : object->getDatas())
			{
				if(ods->getDataRect(data, area))
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
				auto ods = getObjectDrawStruct(object);
				Rect area;

				bool drawData = false;
				const panda::BaseData* data = dynamic_cast<const panda::BaseData*>(event->m_node);
				if(data)
					drawData = ods->getDataRect(data, area);
				if(!drawData)
					area = ods->getVisualArea();

				list.addRectFilled(area, 0x80FF8080);
			}
		}
	}
}
#endif

void GraphView::paintDirtyState(DrawList& list, DrawColors& colors)
{
	for(const auto& ods : m_orderedObjectDrawStructs)
	{
		const auto object = ods->getObject();
		unsigned int fillCol = object->isDirty() ? 0x400000FF : 0x4000FF00;

		auto area = ods->getVisualArea();
		list.addRectFilled(area, fillCol);

		for(panda::BaseData* data : object->getDatas())
		{
			Rect area;
			if(ods->getDataRect(data, area))
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
	Point zoomedMouse = m_viewDelta + localPos / m_zoomFactor;
	if(event->button() == Qt::LeftButton)
	{
		const auto ods = getObjectDrawStructAtPos(zoomedMouse);
		if(ods)
		{
			const auto object = ods->getObject();
			// Testing for Datas first
			Point linkStart;
			panda::BaseData* data = ods->getDataAtPos(zoomedMouse, &linkStart);
			if(data)
			{
				if(!m_pandaDocument->animationIsPlaying())
				{
					// Remove
					if(data->isInput() && data->getParent() && event->modifiers() == Qt::ControlModifier)
					{
						removeLinkTag(data->getParent(), data);
						changeLink(data, nullptr);
					}
					else	// Creating a new Link
					{
						m_clickedData = data;
						computeCompatibleDatas(data);
						m_movingAction = Moving::Link;
						m_previousMousePos = m_currentMousePos = linkStart;

						m_objectsSelection->selectNone();
					}
				}
			}
			else	// No Data, but we still clicked on an object
			{
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
						m_objectsSelection->selectNone();

					m_objectsSelection->setLastSelectedObject(object);
					m_movingAction = Moving::Start;
					m_previousMousePos = zoomedMouse;
				}

				// Maybe do a custom action ?
				if(ods->mousePressEvent(event))
				{
					m_movingAction = Moving::Custom;
					m_capturedDrawStruct = ods;
				}
			}
		}
		else
		{	// Clicked where there is nothing
			if (event->modifiers() == Qt::ControlModifier)
			{
				// Starting a zoom box
				m_movingAction = Moving::ZoomBox;
			}
			else
			{
				// Starting a rubber band to select in a zone
				m_movingAction = Moving::Selection;
			}

			m_objectsSelection->selectNone();
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
			m_previousMousePos = convert(event->globalPos());

			QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
		}
		else
		{
			m_movingAction = Moving::View;
			m_previousMousePos = convert(event->globalPos());

			QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
		}
	}
}

void GraphView::mouseMoveEvent(QMouseEvent* event)
{
	Point localPos = convert(event->localPos());
	Point globalPos = convert(event->globalPos());

	if(m_movingAction == Moving::Start)
	{
		Point mousePos = m_viewDelta + localPos / m_zoomFactor;
		Point delta = mousePos - m_previousMousePos;
		if((delta * m_zoomFactor).norm() > 5)
		{
			m_movingAction = Moving::Object;
			if(m_useMagneticSnap && !m_selectedObjectsDrawStructs.empty())
			{
				auto ods = m_selectedObjectsDrawStructs.back();
				if(ods->acceptsMagneticSnap())
				{
					prepareSnapTargets(ods);
					auto possiblePosition = ods->getPosition() + delta;
					computeSnapDelta(ods, possiblePosition);
					delta = delta + m_snapDelta;
				}
			}
			QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));

			// Remove docked objects from the selection
			m_customSelection.clear();
			for(auto object : m_objectsSelection->get())
			{
				panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
				if(dockable && m_objectsSelection->isSelected(dockable->getParentDock()))
					continue; // don't move a dockable object if their parent dock is selected, it will move them
				m_customSelection.push_back(object);
			}

			m_moveObjectsMacro = m_pandaDocument->getUndoStack().beginMacro(tr("move objects").toStdString());

			if(!delta.isNull())
				m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(this, m_customSelection, delta));

			m_previousMousePos = mousePos;
		}
	}
	else if(m_movingAction == Moving::Object)
	{
		Point mousePos = m_viewDelta + localPos / m_zoomFactor;
		Point delta = mousePos - m_previousMousePos;
		if(m_useMagneticSnap && !m_selectedObjectsDrawStructs.empty())
		{
			Point oldSnapDelta = m_snapDelta;
			auto ods = m_selectedObjectsDrawStructs.back();
			auto possiblePosition = ods->getPosition() + delta - m_snapDelta;
			computeSnapDelta(ods, possiblePosition);
			delta = delta - oldSnapDelta + m_snapDelta;
		}

		if (!m_customSelection.empty() && !delta.isNull())
		{
			m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(this, m_customSelection, delta));
			m_recomputeLinks = true;
		}

		m_previousMousePos = mousePos;
	}
	else if(m_movingAction == Moving::View)
	{
		Point delta = (globalPos - m_previousMousePos) / m_zoomFactor;
		moveView(delta);
		m_previousMousePos = globalPos;
		update();
		emit viewModified();
	}
	else if(m_movingAction == Moving::Zoom)
	{
		int y = event->globalY() - m_previousMousePos.y;
		Point oldPos = m_currentMousePos / m_zoomFactor;
		m_zoomFactor = qBound(0.1, m_zoomFactor - y / 500.0, 1.0);
		m_zoomLevel = 100.0 * (1.0 - m_zoomFactor);
		moveView(m_currentMousePos / m_zoomFactor - oldPos);
		m_previousMousePos = globalPos;
		update();
	}
	else if(m_movingAction == Moving::Selection || m_movingAction == Moving::ZoomBox)
	{
		m_currentMousePos = localPos;
		update();
	}
	else if(m_movingAction == Moving::Link)
	{
		m_currentMousePos = m_viewDelta + localPos / m_zoomFactor;

		const auto ods = getObjectDrawStructAtPos(m_currentMousePos);
		if(ods)
		{
			Point linkStart;
			panda::BaseData* data = ods->getDataAtPos(m_currentMousePos, &linkStart);
			if(data && canLinkWith(data))
				m_currentMousePos = linkStart;
		}

		// Moving the view if the mouse is near the border of the widget
		moveViewIfMouseOnBorder();

		update();
	}
	else if(m_movingAction == Moving::Custom)
	{
		if(m_capturedDrawStruct)
			m_capturedDrawStruct->mouseMoveEvent(event);
	}

	if(m_movingAction == Moving::None || m_movingAction == Moving::Link)
	{
		Point zoomedMouse = m_viewDelta + localPos / m_zoomFactor;
		auto dataRect = getDataAtPos(zoomedMouse);
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

			QString display = QString("%1\n%2")
				.arg(QString::fromStdString(m_hoverData->getName()))
				.arg(QString::fromStdString(m_hoverData->getDescription()));
			QRect area = QRect(dataRect.second.left(), dataRect.second.top(), dataRect.second.width(), dataRect.second.height());
			QToolTip::showText(event->globalPos(), display, this, area);
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

					auto inputData = tag->getInputData();
					auto it = std::find_if(m_dataLabels.begin(), m_dataLabels.end(), [inputData](const auto& dl) {
						return dl.data == inputData;
					});
					QString display;
					if (it != m_dataLabels.end())
						display = QString("<b>%1</b>").arg(QString::fromStdString(it->label));
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
						QToolTip::showText(event->globalPos(), display, this, rect);
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
		{
			m_objectsSelection->selectNone();
			m_objectsSelection->add(object);
		}
	}
	else if(m_movingAction == Moving::Object)
	{
		QMap<panda::PandaObject*, Point> positions;
		for(const auto ods : m_selectedObjectsDrawStructs)
			positions[ods->getObject()] = ods->getPosition();

		for(const auto ods : m_selectedObjectsDrawStructs)
		{
			auto object = ods->getObject();
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			if(dockable)
			{
				Point delta = positions[object] - ods->getPosition();
				m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(this, dockable, delta));

				Rect dockableArea = ods->getSelectionArea();
				panda::DockObject* defaultDock = dockable->getDefaultDock();
				panda::DockObject* newDock = defaultDock;
				int newIndex = -1;
				for(const auto ods2 : m_orderedObjectDrawStructs)
				{
					panda::DockObject* dock = dynamic_cast<panda::DockObject*>(ods2->getObject());
					if(dock)
					{
						if(dockableArea.intersects(ods2->getSelectionArea()) && dock->accepts(dockable))
						{
							newIndex = dynamic_cast<DockObjectDrawStruct*>(ods2)->getDockableIndex(dockableArea);
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

		updateViewRect();
	}
	else if(m_movingAction == Moving::View)
	{
		emit viewModified();
	}
	else if(m_movingAction == Moving::Zoom)
	{
		updateViewRect();
	}
	else if(m_movingAction == Moving::ZoomBox)
	{
		Rect zoomRect = Rect(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor).translated(m_viewDelta).canonicalized();
		if (zoomRect.area() > 1000)
		{
			float factorW = contentsRect().width() / (zoomRect.width() + 40);
			float factorH = contentsRect().height() / (zoomRect.height() + 40);
			m_zoomFactor = panda::helper::bound(0.1f, std::min(factorW, factorH), 1.0f);
			m_zoomLevel = 100 * (1.0 - m_zoomFactor);
			moveView(convert(contentsRect().center()) / m_zoomFactor - zoomRect.center() + m_viewDelta);
		}
		update();
		updateViewRect();
	}
	else if(m_movingAction == Moving::Selection)
	{
		m_objectsSelection->selectNone();

		Rect selectionRect = Rect(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor).translated(m_viewDelta).canonicalized();
		for(const auto ods : m_orderedObjectDrawStructs)
		{
			Rect objectArea = ods->getSelectionArea();
			if(selectionRect.intersects(objectArea))
				m_objectsSelection->add(ods->getObject());
		}

		update();
	}
	else if(m_movingAction == Moving::Link)
	{
		const auto ods = getObjectDrawStructAtPos(m_currentMousePos);
		if(ods)
		{
			panda::BaseData* secondData = ods->getDataAtPos(m_currentMousePos);
			if(secondData && canLinkWith(secondData))
			{
				if(m_clickedData->isInput() && secondData->isOutput())
					changeLink(m_clickedData, secondData);
				else if(secondData->isInput() && m_clickedData->isOutput())
					changeLink(secondData, m_clickedData);
				updateLinkTags();
			}
		}
		m_clickedData = nullptr;
		update();
	}
	else if(m_movingAction == Moving::Custom)
	{
		if(m_capturedDrawStruct)
		{
			m_capturedDrawStruct->mouseReleaseEvent(event);
			m_capturedDrawStruct = nullptr;
			updateViewRect();
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
	int newZoom = qBound(0, m_zoomLevel - ticks, 90);
	if(m_zoomLevel != newZoom)
	{
		Point mousePos = convert(event->pos());
		Point oldPos = mousePos / m_zoomFactor;
		m_zoomLevel = newZoom;
		m_zoomFactor = (100 - m_zoomLevel) / 100.0;
		moveView(mousePos / m_zoomFactor - oldPos);
		update();
		updateViewRect();
	}
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
		{
			moveView(Point(100, 0));
			updateViewRect();
			update();
		}
		break;
	case Qt::Key_Right:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(Point(-100, 0));
			updateViewRect();
			update();
		}
		break;
	case Qt::Key_Up:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(Point(0, 100));
			updateViewRect();
			update();
		}
		break;
	case Qt::Key_Down:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(Point(0, -100));
			updateViewRect();
			update();
		}
		break;
	case Qt::Key_Plus:
		if(event->modifiers() & Qt::ControlModifier)
			zoomIn();
		break;
	case Qt::Key_Minus:
		if(event->modifiers() & Qt::ControlModifier)
			zoomOut();
		break;
	default:
		QWidget::keyPressEvent(event);
	}
}

void GraphView::contextMenuEvent(QContextMenuEvent* event)
{
	m_contextMenuData = nullptr;
	int flags = 0;
	Point zoomedMouse = convert(event->pos()) / m_zoomFactor + m_viewDelta;
	const auto ods = getObjectDrawStructAtPos(zoomedMouse);
	if (ods)
	{
		m_contextMenuObject = ods->getObject();
		flags |= MENU_OBJECT;
		m_contextMenuData = ods->getDataAtPos(zoomedMouse);
		if (m_contextMenuData)
		{
			if (m_contextMenuData->isDisplayed())
				flags |= MENU_DATA;

			if (m_contextMenuData->isInput() && m_contextMenuData->getParent())
				flags |= MENU_LINK;

			const auto trait = m_contextMenuData->getDataTrait();
			if (trait->valueTypeName() == "image")
				flags |= MENU_IMAGE;
		}
	}
	else
		m_contextMenuObject = nullptr;

	m_contextLinkTag = nullptr;
	for (const auto& linkTag : m_linkTags)
	{
		auto dataPair = linkTag->getDataAtPoint(zoomedMouse);
		if (dataPair.first)
		{
			flags |= MENU_TAG;
			m_contextLinkTag = linkTag.get();
			break;
		}
	}

	if(m_hoverTimer->isActive())
		m_hoverTimer->stop();

	emit showContextMenu(event->globalPos(), flags);
}

void GraphView::zoomIn()
{
	if(m_zoomLevel > 0)
	{
		Point center = convert(contentsRect().center());
		Point oldPos = center / m_zoomFactor;
		m_zoomLevel = qMax(m_zoomLevel-10, 0);
		m_zoomFactor = (100 - m_zoomLevel) / 100.0;
		moveView(center / m_zoomFactor - oldPos);
		update();
		updateViewRect();
	}
}

void GraphView::zoomOut()
{
	if(m_zoomLevel < 90)
	{
		Point center = convert(contentsRect().center());
		Point oldPos = center / m_zoomFactor;
		m_zoomLevel = qMin(m_zoomLevel+10, 90);
		m_zoomFactor = (100 - m_zoomLevel) / 100.0;
		moveView(center / m_zoomFactor - oldPos);
		update();
		updateViewRect();
	}
}

void GraphView::zoomReset()
{
	if(m_zoomLevel != 1)
	{
		Point center = convert(contentsRect().center());
		Point oldPos = center / m_zoomFactor;
		m_zoomLevel = 1;
		m_zoomFactor = 1.0;
		moveView(center / m_zoomFactor - oldPos);
		update();
		updateViewRect();
	}
}

void GraphView::centerView()
{
	if(m_objectsList.size())
	{
		Rect totalView;
		for (const auto ods : m_orderedObjectDrawStructs)
		{
			Rect objectArea = ods->getVisualArea();
			totalView = totalView.united(objectArea);
		}

		moveView(convert(contentsRect().center()) / m_zoomFactor - totalView.center() + m_viewDelta);
		update();
		updateViewRect();
	}
}

void GraphView::showAll()
{
	if(m_objectsList.size())
	{
		Rect totalView;
		for (const auto ods : m_orderedObjectDrawStructs)
		{
			Rect objectArea = ods->getVisualArea();
			totalView = totalView.united(objectArea);
		}

		float factorW = contentsRect().width() / (totalView.width() + 40);
		float factorH = contentsRect().height() / (totalView.height() + 40);
		m_zoomFactor = panda::helper::bound(0.1f, std::min(factorW, factorH), 1.0f);
		m_zoomLevel = 100 * (1.0 - m_zoomFactor);
		moveView(convert(contentsRect().center()) / m_zoomFactor - totalView.center() + m_viewDelta);
		update();
		updateViewRect();
	}
}

void GraphView::showAllSelected()
{
	if(!m_objectsSelection->get().empty())
	{
		Rect totalView;
		for (const auto ods : m_orderedObjectDrawStructs)
		{
			Rect objectArea = ods->getVisualArea();
			totalView = totalView.united(objectArea);
		}

		float factorW = contentsRect().width() / (totalView.width() + 40);
		float factorH = contentsRect().height() / (totalView.height() + 40);
		m_zoomFactor = panda::helper::bound(0.1f, std::min(factorW, factorH), 1.0f);
		m_zoomLevel = 100 * (1.0 - m_zoomFactor);
		moveView(convert(contentsRect().center()) / m_zoomFactor - totalView.center() + m_viewDelta);
		update();
		updateViewRect();
	}
}

void GraphView::moveSelectedToCenter()
{
	if(!m_objectsSelection->get().empty())
	{
		Rect totalView;
		for(const auto ods : m_selectedObjectsDrawStructs)
		{
			Rect objectArea = ods->getVisualArea();
			totalView = totalView.united(objectArea);
		}

		Point delta = convert(contentsRect().center()) / m_zoomFactor - totalView.center() + m_viewDelta;

		for(const auto ods : m_selectedObjectsDrawStructs)
		{
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(ods->getObject());
			// Do not move (docked) dockable objects, their parent dock move them already
			if(!dockable || !m_objectsSelection->isSelected(dockable->getParentDock()))
				ods->move(delta);
		}

		update();
		updateViewRect();
	}
}

void GraphView::addedObject(panda::PandaObject* object)
{
	// Creating a DrawStruct depending on the class of the object been added
	// When undoing a delete command, the DrawStruct has already been reinserted
	auto ods = getObjectDrawStruct(object);
	if (!ods)
	{
		auto odsPtr = ObjectDrawStructFactory::getInstance()->createDrawStruct(this, object);
		m_objectDrawStructs.emplace(object, odsPtr);

		ods = odsPtr.get();
		m_orderedObjectDrawStructs.push_back(ods);
	}

	if (!m_dirtyDrawStructsSet.count(ods))
	{
		m_dirtyDrawStructs.push_back(ods);
		m_dirtyDrawStructsSet.insert(ods);
		update();
	}
}

void GraphView::removeObject(panda::PandaObject* object)
{
	auto ods = getObjectDrawStruct(object);
	m_objectDrawStructs.erase(object);
	if(ods)
		panda::helper::removeOne(m_orderedObjectDrawStructs, ods);
	m_capturedDrawStruct = nullptr;
	m_movingAction = Moving::None;
	m_linkTags.clear();
	m_linkTagsMap.clear();
	m_recomputeTags = true;
	m_highlightConnectedDatas = false;
	update();
	updateViewRect();
}

void GraphView::modifiedObject(panda::PandaObject* object)
{
	m_recomputeLinks = true;
	auto ods = getObjectDrawStruct(object);
	if(ods)	// Can be called before the object is fully created
	{
		panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object);
		if (dock)
		{
			auto dods = dynamic_cast<DockObjectDrawStruct*>(ods);
			if (dods)
				dods->placeDockableObjects();
		}

		if (!m_dirtyDrawStructsSet.count(ods))
		{
			m_dirtyDrawStructs.push_back(ods);
			m_dirtyDrawStructsSet.insert(ods);
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
				auto ownerOds = getObjectDrawStruct(data->getOwner());
				auto parentOds = getObjectDrawStruct(parentData->getOwner());
				if (ownerOds && parentOds)
				{
					Rect ownerRect, parentRect;
					ownerOds->getDataRect(data, ownerRect);
					parentOds->getDataRect(parentData, parentRect);
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

	for(const auto ods : m_orderedObjectDrawStructs)
	{
		for (const auto& toDataRect : ods->getDataRects())
		{
			panda::BaseData* data = toDataRect.first;
			panda::BaseData* parent = data->getParent();
			if (parent && !data->isOutput())
			{
				Rect fromDataRect;
				auto ods = getObjectDrawStruct(parent->getOwner());
				if (ods && ods->getDataRect(parent, fromDataRect) 
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

	std::vector<Rect> highlightRects;
	std::vector< std::pair<Point, Point> > highlightLinks;

	Rect sourceRect;
	if(getObjectDrawStruct(m_hoverData->getOwner())->getDataRect(m_hoverData, sourceRect))
		highlightRects.push_back(sourceRect);
	else
		return;

	// Get outputs
	if(m_hoverData->isOutput())
	{
		for(const auto node : m_hoverData->getOutputs())
		{
			panda::BaseData* data = dynamic_cast<panda::BaseData*>(node);
			if(data)
			{
				panda::PandaObject* object = data->getOwner();
				auto ods = getObjectDrawStruct(object);
				if(ods)
				{
					Rect rect;
					if(ods->getDataRect(data, rect))
					{
						highlightRects.push_back(rect);
						highlightLinks.emplace_back(rect.center(), sourceRect.center());
					}
				}
			}
		}
	}
	// Or the one input
	else if(m_hoverData->isInput())
	{
		panda::BaseData* data = m_hoverData->getParent();
		if(data)
		{
			panda::PandaObject* object = data->getOwner();
			auto ods = getObjectDrawStruct(object);
			if (ods)
			{
				Rect rect;
				if(ods->getDataRect(data, rect))
				{
					highlightRects.push_back(rect);
					highlightLinks.emplace_back(sourceRect.center(), rect.center());
				}
			}
		}
	}

	if(highlightLinks.empty())
		return;

	// Now draw everything
	auto highlightCol = DrawList::convert(palette().highlight().color());
	auto penCol = DrawList::convert(palette().text().color());

	for (const auto& rect : highlightRects)
	{
		m_connectedDrawList.addRectFilled(rect, highlightCol);
		m_connectedDrawList.addRect(rect, penCol, 1.f);
	}

	for(const auto& link : highlightLinks)
	{
		float w = (link.second.x - link.first.x) / 2;
		auto p1 = link.first, p2 = link.second, d = Point(w, 0);
		m_connectedDrawList.addBezierCurve(p1, p1 + d, p2 - d, p2, highlightCol, 3);
	}
}

void GraphView::prepareSnapTargets(ObjectDrawStruct* selectedDrawStruct)
{
	m_snapTargetsY.clear();

	float y = selectedDrawStruct->getPosition().y;
	// For y, try to make the data links horizontal
	// First for inputs
	for(auto input : selectedDrawStruct->getObject()->getInputDatas())
	{
		Rect dataRect;
		if(selectedDrawStruct->getDataRect(input, dataRect))
		{
			auto dataHeight = dataRect.center().y - y;
			for(auto input2 : input->getInputs())
			{
				auto data2 = dynamic_cast<panda::BaseData*>(input2);
				if(data2 && data2->getOwner())
				{
					auto owner = data2->getOwner();
					auto ods = getObjectDrawStruct(owner);
					if(ods)
					{
						if(ods->getDataRect(data2, dataRect))
							m_snapTargetsY.insert(dataRect.center().y - dataHeight);
					}
				}
			}
		}
	}

	// Then for outputs
	for(auto output : selectedDrawStruct->getObject()->getOutputDatas())
	{
		Rect dataRect;
		if(selectedDrawStruct->getDataRect(output, dataRect))
		{
			auto dataHeight = dataRect.center().y - y;
			for(auto output2 : output->getOutputs())
			{
				auto data2 = dynamic_cast<panda::BaseData*>(output2);
				if(data2 && data2->getOwner())
				{
					auto owner = data2->getOwner();
					auto ods = getObjectDrawStruct(owner);
					if (ods)
					{
						if(ods->getDataRect(data2, dataRect))
							m_snapTargetsY.insert(dataRect.center().y - dataHeight);
					}
				}
			}
		}
	}
}

void GraphView::computeSnapDelta(ObjectDrawStruct* selectedDrawStruct, Point position)
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
	auto selectedHeight = selectedDrawStruct->getObjectSize().y;
	Rect viewRect(m_viewDelta, width() / m_zoomFactor, height() / m_zoomFactor);
	auto m1 = std::numeric_limits<float>::lowest(), m2 = std::numeric_limits<float>::max();
	Point abovePos(m1, m1), belowPos(m2, m2);
	float aboveDist{ m2 }, belowDist{ m2 };
	bool hasInsideObject = false;
	std::set<float> snapTargetsX;
	for (const auto ods : m_orderedObjectDrawStructs)
	{
		if (ods == selectedDrawStruct || !ods->acceptsMagneticSnap())
			continue;

		auto area = ods->getVisualArea();
		if (viewRect.intersects(area)) // Only if visible in the current viewport
		{
			auto pos = ods->getPosition();
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
		auto ods = getObjectDrawStruct(object);
		if(ods)
			ods->move(delta);
	}
	emit modified();
	updateLinkTags();
	m_recomputeLinks = true;
	update();
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
		auto lpos = getObjectDrawStruct(lhs)->getPosition();
		auto rpos = getObjectDrawStruct(rhs)->getPosition();
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
	updateDirtyDrawStructs();
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
	auto delta = m_viewDelta * m_zoomFactor;
	return QPoint(m_viewRect.left() - delta.x, m_viewRect.top() - delta.y);
}

void GraphView::scrollView(QPoint position)
{
	Point delta = convert(position) - m_viewRect.topLeft() + m_viewDelta * m_zoomFactor;
	moveView(delta);
	update();
}

void GraphView::updateViewRect()
{
	if(m_isLoading)
		return;

	m_objectsRect = m_viewRect = Rect();
	for(const auto& ods : m_orderedObjectDrawStructs)
	{
		Rect area = ods->getVisualArea();
		m_objectsRect |= area;
		Rect zoomedArea = Rect::fromSize(area.topLeft() * m_zoomFactor, area.size() * m_zoomFactor);
		m_viewRect |= zoomedArea; // Union
	}

	if(!m_orderedObjectDrawStructs.empty())
		m_viewRect.adjust(-5, -5, 5, 5);

	emit viewModified();
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
	m_selectedObjectsDrawStructs.clear();

	for (auto object : m_objectsSelection->get())
	{
		auto ods = getObjectDrawStruct(object);
		if(ods)
			m_selectedObjectsDrawStructs.push_back(ods);
	}

	update();
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

void GraphView::updateDirtyDrawStructs()
{
	if (m_dirtyDrawStructs.empty())
		return;

	for (auto ods : m_dirtyDrawStructs)
		ods->update();
	m_dirtyDrawStructs.clear();
	m_dirtyDrawStructsSet.clear();

	m_linkTags.clear();
	m_linkTagsMap.clear();
	m_recomputeTags = true;
	m_highlightConnectedDatas = false;
	m_hoverData = nullptr;
	m_hoverTimer->stop();
	m_recomputeLinks = true;
	m_recomputeConnected = true;
	updateViewRect();
}

void GraphView::objectsReordered()
{
	m_orderedObjectDrawStructs.clear();
	for (const auto& obj : m_objectsList.get())
		m_orderedObjectDrawStructs.push_back(getObjectDrawStruct(obj.get()));
}

panda::types::Point GraphView::getNewObjectPosition()
{
	return convert(contentsRect().center()) + m_viewDelta;
}

void GraphView::setLinkTagName()
{
	if (!m_contextLinkTag)
		return;

	auto data = m_contextLinkTag->getInputData();
	auto it = std::find_if(m_dataLabels.begin(), m_dataLabels.end(), [data](const auto& dl) {
		return dl.data == data;
	});

	std::string label;
	if (it != m_dataLabels.end())
		label = it->label;

	bool ok = false;
	label = QInputDialog::getMultiLineText(this, tr("Data label"), tr("Label:"), QString::fromStdString(label), &ok).toStdString();
	if (!ok)
		return;

	// Remove the label
	if (label.empty())
	{
		if (it != m_dataLabels.end())
			m_dataLabels.erase(it);
	}
	else
	{
		// Modify the label
		if (it != m_dataLabels.end())
			it->label = label;
		else // Add a label
		{
			DataLabel dl;
			dl.data = data;
			dl.object = data->getOwner();
			dl.label = label;
			m_dataLabels.push_back(dl);
		}
	}

	emit modified();
}

void GraphView::moveViewIfMouseOnBorder()
{
	auto localPos = (m_currentMousePos - m_viewDelta) * m_zoomFactor;
	auto contents = contentsRect();
	auto area = Rect(convert(contents.topLeft()), convert(contents.bottomRight()));
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
			const float speed = 10.f / m_zoomFactor;
			m_previousTime = now;
			Point delta = speed * dt * Point(dx, dy);
			moveView(delta);
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
	moveSelectedToCenter();

	auto view = isTemporaryView() ? nullptr : this;
	m_pandaDocument->getUndoStack().push(std::make_shared<AddObjectCommand>(m_pandaDocument, m_objectsList, view, result.second));
}

void GraphView::del()
{
	const auto& selection = m_objectsSelection->get();
	if (selection.empty())
		return;

	auto view = isTemporaryView() ? nullptr : this;
	auto macro = m_pandaDocument->getUndoStack().beginMacro(tr("delete objects").toStdString());	
	m_pandaDocument->getUndoStack().push(std::make_shared<RemoveObjectCommand>(m_pandaDocument, m_objectsList, view, selection));
}