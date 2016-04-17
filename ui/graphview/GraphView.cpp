#include <QtWidgets>
#include <cmath>
#include <functional>
#include <limits>

#include <ui/command/MoveObjectCommand.h>
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

#ifdef PANDA_LOG_EVENTS
#include <ui/dialog/UpdateLoggerDialog.h>
#endif

using pPoint = panda::types::Point;

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
}

static inline panda::types::Point convert(const QPointF& pt)
{ return panda::types::Point(static_cast<float>(pt.x()), static_cast<float>(pt.y())); }

GraphView::GraphView(panda::PandaDocument* doc, QWidget* parent)
	: QOpenGLWidget(parent)
	, m_pandaDocument(doc)
	, m_hoverTimer(new QTimer(this))
	, m_objectsSelection(std::make_unique<ObjectsSelection>(doc))
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
	m_observer.get(docSignals.addedObject).connect<GraphView, &GraphView::addedObject>(this);
	m_observer.get(docSignals.removedObject).connect<GraphView, &GraphView::removeObject>(this);
	m_observer.get(docSignals.modifiedObject).connect<GraphView, &GraphView::modifiedObject>(this);
	m_observer.get(docSignals.savingObject).connect<GraphView, &GraphView::savingObject>(this);
	m_observer.get(docSignals.loadingObject).connect<GraphView, &GraphView::loadingObject>(this);
	m_observer.get(docSignals.startLoading).connect<GraphView, &GraphView::startLoading>(this);
	m_observer.get(docSignals.loadingFinished).connect<GraphView, &GraphView::loadingFinished>(this);
	m_observer.get(docSignals.changedDock).connect<GraphView, &GraphView::changedDock>(this);
	m_observer.get(docSignals.reorderedObjects).connect<GraphView, &GraphView::objectsReordered>(this);

	connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverDataInfo()));

	m_hoverTimer->setSingleShot(true);

	setMouseTracking(true);

	const auto& pal = palette();
	m_drawColors.penColor = DrawList::convert(pal.text().color());
	m_drawColors.midLightColor = DrawList::convert(pal.midlight().color());
	m_drawColors.lightColor = DrawList::convert(pal.light().color());
	m_drawColors.highlightColor = DrawList::convert(pal.highlight().color());
}

GraphView::~GraphView() = default;

void GraphView::resetView()
{
	m_viewDelta = QPointF();
	m_zoomLevel = 0;
	m_zoomFactor = 1.0;
	m_objectDrawStructs.clear();
	m_orderedObjectDrawStructs.clear();
	m_linkTags.clear();
	m_movingAction = MOVING_NONE;
	m_clickedData = nullptr;
	m_hoverData = nullptr;
	m_contextMenuData = nullptr;
	m_capturedDrawStruct = nullptr;
	m_recomputeTags = false;
	m_hoverTimer->stop();
	m_highlightConnectedDatas = false;
	m_useMagneticSnap = true;
	m_isLoading = false;
	m_viewRect = QRect();
	m_selectedObjectsDrawStructs.clear();
	m_possibleLinks.clear();
	m_dirtyDrawStructs.clear();
	m_dirtyDrawStructsSet.clear();

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

ObjectDrawStruct* GraphView::getObjectDrawStructAtPos(const QPointF& pt)
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

QRectF GraphView::getDataRect(panda::BaseData* data)
{
	QRectF rect;
	ObjectDrawStruct* ods = getObjectDrawStruct(data->getOwner());
	if(ods)
		ods->getDataRect(data, rect);
	return rect;
}

void GraphView::moveView(const QPointF& delta)
{
	if(!delta.isNull())
	{
		m_viewDelta += delta;
		for(auto& drawStruct : m_orderedObjectDrawStructs)
			drawStruct->moveVisual(delta);

		for(auto& tag : m_linkTags)
			tag.second->moveView(delta);
	}
}

void GraphView::initializeGL()
{
	m_viewRenderer->initialize();
	m_linksDrawList = DrawList();
	m_connectedDrawList = DrawList();
	m_selectedObjectsDrawList = DrawList();
}

void GraphView::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	m_viewRenderer->resize(w, h);
	update();
}

void GraphView::paintGL()
{
	updateDirtyDrawStructs();

	if(m_recomputeTags)
	{
		updateLinkTags();
		m_recomputeTags = false;
	}

	m_viewRenderer->setView({ 0, 0, width() / static_cast<float>(m_zoomFactor), height() / static_cast<float>(m_zoomFactor) });
	m_viewRenderer->newFrame();
	DrawList drawList;

	QPainter painter(this);

	painter.beginNativePainting();
	auto col = palette().background().color();
	glClearColor(col.redF(), col.greenF(), col.blueF(), 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	painter.endNativePainting();

	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	// Zoom
	painter.scale(m_zoomFactor, m_zoomFactor);
/*
	// Give a possibility to draw behind normal objects
	for (auto& ods : m_orderedObjectDrawStructs)
		ods->drawBackground(&painter);

	// Draw links
	drawLinks();

	// Draw the objects
	for (auto& ods : m_orderedObjectDrawStructs)
		ods->draw(&painter);

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for (auto& ods : m_selectedObjectsDrawStructs)
		ods->draw(&painter, true);

	painter.setBrush(Qt::NoBrush);
	// Give a possibility to draw in front of normal objects
	for (auto& ods : m_orderedObjectDrawStructs)
		ods->drawForeground(&painter);
*/
	// Draw links tags
	for (auto& tag : m_linkTags)
		tag.second->draw(&painter);

	// Highlight connected Datas
	drawConnectedDatas(m_hoverData);

	// Selection rubber band
	if (m_movingAction == MOVING_SELECTION)
	{
		QRectF r = QRectF(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor).normalized();
		auto highlight = palette().highlight().color();
		highlight.setAlpha(64);
		drawList.addRectFilled(convert(r.topLeft()), convert(r.bottomRight()), DrawList::convert(highlight));
		drawList.addRect(convert(r.topLeft()), convert(r.bottomRight()), DrawList::convert(palette().text().color()));
	}

	// Link in creation
	if (m_movingAction == MOVING_LINK)
	{
		drawList.addLine(convert(m_previousMousePos), convert(m_currentMousePos),
						 DrawList::convert(palette().text().color()), 1.5);
	}

	if (m_debugDirtyState)
	{
#ifdef PANDA_LOG_EVENTS
		UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
		if(logDlg && logDlg->isVisible())
			paintLogDebug(painter);
		else
#endif
			paintDirtyState(painter);
	}

	painter.end();

	// Give a possibility to draw behind normal objects
	for (auto& ods : m_orderedObjectDrawStructs)
		ods->drawBackground(drawList, m_drawColors);

	// Draw links
	drawLinks();

	// Draw the objects
	for (auto& ods : m_orderedObjectDrawStructs)
		ods->draw(drawList, m_drawColors);

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for (auto& ods : m_selectedObjectsDrawStructs)
		ods->draw(drawList, m_drawColors, true);

	// Give a possibility to draw in front of normal objects
	for (auto& ods : m_orderedObjectDrawStructs)
		ods->drawForeground(drawList, m_drawColors);

	m_viewRenderer->addDrawList(&m_linksDrawList);
	m_viewRenderer->addDrawList(&drawList);
	m_viewRenderer->addDrawList(&m_connectedDrawList);
	m_viewRenderer->render();
}

#ifdef PANDA_LOG_EVENTS
void GraphView::paintLogDebug(QPainter& painter)
{
	UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
	if(logDlg && logDlg->isVisible())
	{
		const auto& states = logDlg->getNodeStates();
		for(const auto ods : m_orderedObjectDrawStructs)
		{
			const auto object = ods->getObject();
			if(panda::helper::valueOrDefault(states, object, nullptr))
				painter.setBrush(QColor(255,0,0,32));
			else
				painter.setBrush(QColor(0,255,0,32));

			QRectF area = ods->getObjectArea();
			painter.drawRect(area);

			for(panda::BaseData* data : object->getDatas())
			{
				if(ods->getDataRect(data, area))
				{
					if(panda::helper::valueOrDefault(states, data, nullptr))
						painter.setBrush(QColor(255,0,0,64));
					else
						painter.setBrush(QColor(0,255,0,64));

					painter.drawRect(area);
				}
			}
		}

		const panda::helper::EventData* event = logDlg->getSelectedEvent();
		if(event)
		{
			panda::PandaObject* object = m_pandaDocument->findObject(event->m_objectIndex);
			if(object)
			{
				auto ods = getObjectDrawStruct(object);
				painter.setBrush(QColor(128, 128, 255, 128));
				QRectF area;

				bool drawData = false;
				const panda::BaseData* data = dynamic_cast<const panda::BaseData*>(event->m_node);
				if(data)
					drawData = ods->getDataRect(data, area);
				if(!drawData)
					area = ods->getObjectArea();

				painter.drawRect(area);
			}
		}
	}
}
#endif

void GraphView::paintDirtyState(QPainter& painter)
{
	for(const auto& ods : m_orderedObjectDrawStructs)
	{
		const auto object = ods->getObject();
		if(object->isDirty())
			painter.setBrush(QColor(255,0,0,64));
		else
			painter.setBrush(QColor(0,255,0,64));
		painter.drawRect(ods->getObjectArea());

		for(panda::BaseData* data : object->getDatas())
		{
			QRectF area;
			if(ods->getDataRect(data, area))
			{
				if(data->isDirty())
					painter.setBrush(QColor(255,0,0,128));
				else
					painter.setBrush(QColor(0,255,0,128));
				painter.drawRect(area);
			}
		}
	}
}

void GraphView::mousePressEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / m_zoomFactor;
	if(event->button() == Qt::LeftButton)
	{
		const auto ods = getObjectDrawStructAtPos(zoomedMouse);
		if(ods)
		{
			const auto object = ods->getObject();
			// Testing for Datas first
			QPointF linkStart;
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
						m_movingAction = MOVING_LINK;
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
					m_movingAction = MOVING_START;
					m_previousMousePos = zoomedMouse;
				}

				// Maybe do a custom action ?
				if(ods->mousePressEvent(event))
				{
					m_movingAction = MOVING_CUSTOM;
					m_capturedDrawStruct = ods;
				}
			}
		}
		else
		{
			// Clicked where there is nothing
			// Starting a rubber band to select in a zone
			m_objectsSelection->selectNone();
			m_movingAction = MOVING_SELECTION;
			m_previousMousePos = m_currentMousePos = event->localPos();
			QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
		}
	}
	else if(event->button() == Qt::MidButton)
	{
		if(event->modifiers() == Qt::ControlModifier)
		{
			m_movingAction = MOVING_ZOOM;
			m_currentMousePos = event->pos();
			m_previousMousePos = event->globalPos();

			QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
		}
		else
		{
			m_movingAction = MOVING_VIEW;
			m_previousMousePos = event->globalPos();

			QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
		}
	}
}

void GraphView::mouseMoveEvent(QMouseEvent* event)
{
	if(m_movingAction == MOVING_START)
	{
		QPointF mousePos = event->localPos() / m_zoomFactor;
		QPointF delta = mousePos - m_previousMousePos;
		if(QVector2D(delta *  m_zoomFactor).length() > 5)
		{
			m_movingAction = MOVING_OBJECT;
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
	else if(m_movingAction == MOVING_OBJECT)
	{
		QPointF mousePos = event->localPos() / m_zoomFactor;
		QPointF delta = mousePos - m_previousMousePos;
		if(m_useMagneticSnap && !m_selectedObjectsDrawStructs.empty())
		{
			QPointF oldSnapDelta = m_snapDelta;
			auto ods = m_selectedObjectsDrawStructs.back();
			auto possiblePosition = ods->getPosition() + delta - m_snapDelta;
			computeSnapDelta(ods, possiblePosition);
			delta = delta - oldSnapDelta + m_snapDelta;
		}

		if(!m_customSelection.empty() && !delta.isNull())
			m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(this, m_customSelection, delta));

		m_previousMousePos = mousePos;
	}
	else if(m_movingAction == MOVING_VIEW)
	{
		QPointF delta = (event->globalPos() - m_previousMousePos) / m_zoomFactor;
		moveView(delta);
		m_previousMousePos = event->globalPos();
		update();
	}
	else if(m_movingAction == MOVING_ZOOM)
	{
		int y = event->globalY() - m_previousMousePos.y();
		QPointF oldPos = m_currentMousePos / m_zoomFactor;
		m_zoomFactor = qBound(0.1, m_zoomFactor - y / 500.0, 1.0);
		m_zoomLevel = 100.0 * (1.0 - m_zoomFactor);
		moveView(m_currentMousePos / m_zoomFactor - oldPos);
		m_previousMousePos = event->globalPos();
		update();
	}
	else if(m_movingAction == MOVING_SELECTION)
	{
		m_currentMousePos = event->localPos();
		update();
	}
	else if(m_movingAction == MOVING_LINK)
	{
		m_currentMousePos = event->localPos() / m_zoomFactor;

		const auto ods = getObjectDrawStructAtPos(m_currentMousePos);
		if(ods)
		{
			QPointF linkStart;
			panda::BaseData* data = ods->getDataAtPos(m_currentMousePos, &linkStart);
			if(data && canLinkWith(data))
				m_currentMousePos = linkStart;
		}
		update();
	}
	else if(m_movingAction == MOVING_CUSTOM)
	{
		if(m_capturedDrawStruct)
			m_capturedDrawStruct->mouseMoveEvent(event);
	}

	if(m_movingAction == MOVING_NONE || m_movingAction == MOVING_LINK)
	{
		QPointF zoomedMouse = event->localPos() / m_zoomFactor;
		const auto ods = getObjectDrawStructAtPos(zoomedMouse);
		if(ods)
		{
			panda::BaseData* data = ods->getDataAtPos(zoomedMouse);
			if(m_hoverData != data)
			{
				m_hoverTimer->stop();
				if(m_highlightConnectedDatas)
				{
					m_highlightConnectedDatas = false;
					update();
				}
				m_hoverData = data;
				if(m_hoverData)
					m_hoverTimer->start(500);
			}

			if(m_hoverData)
			{
				QRectF dataRect;
				if(ods->getDataRect(m_hoverData, dataRect))
				{
					QString display = QString("%1\n%2")
							.arg(QString::fromStdString(m_hoverData->getName()))
							.arg(QString::fromStdString(m_hoverData->getDescription()));
					QToolTip::showText(event->globalPos(), display, this, dataRect.toRect());
					if(!m_hoverData->getHelp().empty())
						emit showStatusBarMessage(QString::fromStdString(m_hoverData->getHelp()));
				}
			}
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

			if(m_movingAction == MOVING_NONE)
			{
				// Look for link tags
				for(auto& tagPair : m_linkTags)
				{
					auto& tag = tagPair.second;
					bool hover = tag->containsPoint(zoomedMouse);
					if(hover != tag->isHovering())
					{
						tag->setHovering(hover);
						update();
					}
				}
			}
		}
	}
}

void GraphView::mouseReleaseEvent(QMouseEvent* event)
{
	if(m_movingAction == MOVING_START)
	{
		panda::PandaObject* object = m_objectsSelection->lastSelectedObject();
		if(object)
		{
			m_objectsSelection->selectNone();
			m_objectsSelection->add(object);
		}
	}
	else if(m_movingAction == MOVING_OBJECT)
	{
		QMap<panda::PandaObject*, QPointF> positions;
		for(const auto ods : m_selectedObjectsDrawStructs)
			positions[ods->getObject()] = ods->getPosition();

		for(const auto ods : m_selectedObjectsDrawStructs)
		{
			auto object = ods->getObject();
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			if(dockable)
			{
				QPointF delta = positions[object] - ods->getPosition();
				m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(this, dockable, delta));

				QRectF dockableArea = ods->getObjectArea();
				panda::DockObject* defaultDock = dockable->getDefaultDock();
				panda::DockObject* newDock = defaultDock;
				int newIndex = -1;
				for(const auto ods2 : m_orderedObjectDrawStructs)
				{
					panda::DockObject* dock = dynamic_cast<panda::DockObject*>(ods2->getObject());
					if(dock)
					{
						if(dockableArea.intersects(ods2->getObjectArea()) && dock->accepts(dockable))
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
	else if(m_movingAction == MOVING_VIEW)
	{
		updateViewRect();
	}
	else if(m_movingAction == MOVING_ZOOM)
	{
		updateViewRect();
	}
	else if(m_movingAction == MOVING_SELECTION)
	{
		m_objectsSelection->selectNone();

		QRectF selectionRect = QRectF(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor).normalized();
		for(const auto ods : m_orderedObjectDrawStructs)
		{
			QRectF objectArea = ods->getObjectArea();
			if(selectionRect.contains(objectArea) || selectionRect.intersects(objectArea))
				m_objectsSelection->add(ods->getObject());
		}

		update();
	}
	else if(m_movingAction == MOVING_LINK)
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
	else if(m_movingAction == MOVING_CUSTOM)
	{
		if(m_capturedDrawStruct)
		{
			m_capturedDrawStruct->mouseReleaseEvent(event);
			m_capturedDrawStruct = nullptr;
			updateViewRect();
		}
	}

	QApplication::restoreOverrideCursor();
	m_movingAction = MOVING_NONE;
}

void GraphView::wheelEvent(QWheelEvent* event)
{
	if(m_movingAction != MOVING_NONE)
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
		QPointF mousePos = event->pos();
		QPointF oldPos = mousePos / m_zoomFactor;
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
			moveView(QPointF(100, 0));
			updateViewRect();
			update();
		}
		break;
	case Qt::Key_Right:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(QPointF(-100, 0));
			updateViewRect();
			update();
		}
		break;
	case Qt::Key_Up:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(QPointF(0, 100));
			updateViewRect();
			update();
		}
		break;
	case Qt::Key_Down:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(QPointF(0, -100));
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
	QPointF zoomedMouse = event->pos() / m_zoomFactor;
	const auto ods = getObjectDrawStructAtPos(zoomedMouse);
	if(ods)
	{
		flags |= MENU_OBJECT;
		m_contextMenuData = ods->getDataAtPos(zoomedMouse);
		if(m_contextMenuData)
		{
			if(m_contextMenuData->isDisplayed())
				flags |= MENU_DATA;

			if(m_contextMenuData->isInput() && m_contextMenuData->getParent())
				flags |= MENU_LINK;

			const auto trait = m_contextMenuData->getDataTrait();
			if(trait->valueTypeName() == "image")
				flags |= MENU_IMAGE;
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
		QPointF center = contentsRect().center();
		QPointF oldPos = center / m_zoomFactor;
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
		QPointF center = contentsRect().center();
		QPointF oldPos = center / m_zoomFactor;
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
		QPointF center = contentsRect().center();
		QPointF oldPos = center / m_zoomFactor;
		m_zoomLevel = 1;
		m_zoomFactor = 1.0;
		moveView(center / m_zoomFactor - oldPos);
		update();
		updateViewRect();
	}
}

void GraphView::centerView()
{
	if(m_pandaDocument->getNbObjects())
	{
		QRectF totalView;
		for (const auto ods : m_orderedObjectDrawStructs)
		{
			QRectF objectArea = ods->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		moveView(contentsRect().center() / m_zoomFactor - totalView.center());
		update();
		updateViewRect();
	}
}

void GraphView::showAll()
{
	if(m_pandaDocument->getNbObjects())
	{
		QRectF totalView;
		for (const auto ods : m_orderedObjectDrawStructs)
		{
			QRectF objectArea = ods->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		qreal factorW = contentsRect().width() / (totalView.width() + 40);
		qreal factorH = contentsRect().height() / (totalView.height() + 40);
		m_zoomFactor = qBound(0.1, qMin(factorW, factorH), 1.0);
		m_zoomLevel = 100 * (1.0 - m_zoomFactor);
		moveView(contentsRect().center() / m_zoomFactor - totalView.center());
		update();
		updateViewRect();
	}
}

void GraphView::showAllSelected()
{
	if(!m_objectsSelection->get().empty())
	{
		QRectF totalView;
		for (const auto ods : m_orderedObjectDrawStructs)
		{
			QRectF objectArea = ods->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		qreal factorW = contentsRect().width() / (totalView.width() + 40);
		qreal factorH = contentsRect().height() / (totalView.height() + 40);
		m_zoomFactor = qBound(0.1, qMin(factorW, factorH), 1.0);
		m_zoomLevel = 100 * (1.0 - m_zoomFactor);
		moveView(contentsRect().center() / m_zoomFactor - totalView.center());
		update();
		updateViewRect();
	}
}

void GraphView::moveSelectedToCenter()
{
	if(!m_objectsSelection->get().empty())
	{
		QRectF totalView;
		for(const auto ods : m_selectedObjectsDrawStructs)
		{
			QRectF objectArea = ods->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		QPointF delta = contentsRect().center() / m_zoomFactor - totalView.center();

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
	m_movingAction = MOVING_NONE;
	m_linkTags.clear();
	m_recomputeTags = true;
	m_highlightConnectedDatas = false;
	update();
	updateViewRect();
}

void GraphView::modifiedObject(panda::PandaObject* object)
{
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

void GraphView::savingObject(panda::XmlElement& elem, panda::PandaObject* object)
{
	getObjectDrawStruct(object)->save(elem);
}

void GraphView::loadingObject(const panda::XmlElement& elem, panda::PandaObject* object)
{
	getObjectDrawStruct(object)->load(elem);
}

int GraphView::getAvailableLinkTagIndex()
{
	int nb = m_linkTags.size();
	std::vector<bool> indices(nb, true);

	for(const auto& tagPair : m_linkTags)
	{
		int id = tagPair.second->index();
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
	if(m_linkTags.count(input))
		m_linkTags[input]->addOutput(output);
	else
		m_linkTags[input] = std::make_shared<LinkTag>(this, input, output, getAvailableLinkTagIndex());
}

void GraphView::removeLinkTag(panda::BaseData* input, panda::BaseData* output)
{
	if(m_linkTags.count(input))
	{
		LinkTag* tag = m_linkTags[input].get();
		tag->removeOutput(output);
		if(tag->isEmpty())
			m_linkTags.erase(input);
	}
}

void GraphView::updateLinkTags(bool reset)
{
	if(reset)
		m_linkTags.clear();

	for(auto& object : m_pandaDocument->getObjects())
	{
		for(auto& data : object->getInputDatas())
		{
			panda::BaseData* parentData = data->getParent();
			if(parentData)
			{
				qreal ox = getDataRect(data).center().x();
				qreal ix = getDataRect(parentData).center().x();
				if(LinkTag::needLinkTag(ix, ox, this))
					addLinkTag(parentData, data);
			}
		}
	}

	// Updating tags and removing empty ones
	for (auto it = m_linkTags.begin(); it != m_linkTags.end();)
	{
		it->second->update();
		if (it->second->isEmpty())
			it = m_linkTags.erase(it);
		else
			++it;
	}

	// Updating the connections list
	m_linkTagsDatas.clear();
	for (const auto& it : m_linkTags)
	{
		const auto& linkTag = it.second;
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
		update();
	}
}

void GraphView::drawLinks()
{
	m_linksDrawList.clear();

	auto col = DrawList::convert(palette().text().color());

	for(const auto ods : m_orderedObjectDrawStructs)
	{
		for (const auto& toDataRect : ods->getDataRects())
		{
			panda::BaseData* data = toDataRect.second;
			panda::BaseData* parent = data->getParent();
			if (parent && !data->isOutput())
			{
				QRectF fromDataRect;
				auto ods = getObjectDrawStruct(parent->getOwner());
				if (ods && ods->getDataRect(parent, fromDataRect) 
					&& !hasLinkTag(parent, data)) // We don't draw the link if there is a LinkTag
				{
					auto d1 = convert(fromDataRect.center()), d2 = convert(toDataRect.first.center());
					pPoint w = { (d2.x - d1.x) / 2, 0 };
					m_linksDrawList.addBezierCurve(d1, d1 + w, d2 - w, d2, col, 1);
				}
			}
		}
	}
}

void GraphView::drawConnectedDatas(panda::BaseData* sourceData)
{
	m_connectedDrawList.clear();
	if (!m_highlightConnectedDatas)
		return;

	std::vector<QRectF> highlightRects;
	std::vector< std::pair<QPointF, QPointF> > highlightLinks;

	QRectF sourceRect;
	if(getObjectDrawStruct(sourceData->getOwner())->getDataRect(sourceData, sourceRect))
		highlightRects.push_back(sourceRect);
	else
		return;

	// Get outputs
	if(sourceData->isOutput())
	{
		for(const auto node : sourceData->getOutputs())
		{
			panda::BaseData* data = dynamic_cast<panda::BaseData*>(node);
			if(data)
			{
				panda::PandaObject* object = data->getOwner();
				auto ods = getObjectDrawStruct(object);
				if(ods)
				{
					QRectF rect;
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
	else if(sourceData->isInput())
	{
		panda::BaseData* data = sourceData->getParent();
		if(data)
		{
			panda::PandaObject* object = data->getOwner();
			auto ods = getObjectDrawStruct(object);
			if (ods)
			{
				QRectF rect;
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
		auto tl = convert(rect.topLeft()), br = convert(rect.bottomRight());
		m_connectedDrawList.addRectFilled(tl, br, highlightCol);
		m_connectedDrawList.addRect(tl, br, penCol, 1.f);
	}

	for(const auto& link : highlightLinks)
	{
		double w = (link.second.x() - link.first.x()) / 2;
		auto p1 = convert(link.first), p2 = convert(link.second), d = pPoint(w, 0);
		m_connectedDrawList.addBezierCurve(p1, p1 + d, p2 - d, p2, highlightCol, 3);
	}
}

void GraphView::prepareSnapTargets(ObjectDrawStruct* selectedDrawStruct)
{
	m_snapTargetsY.clear();

	qreal y = selectedDrawStruct->getPosition().y();
	// For y, try to make the data links horizontal
	// First for inputs
	for(auto input : selectedDrawStruct->getObject()->getInputDatas())
	{
		QRectF dataRect;
		if(selectedDrawStruct->getDataRect(input, dataRect))
		{
			auto dataHeight = dataRect.center().y() - y;
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
							m_snapTargetsY.insert(dataRect.center().y() - dataHeight);
					}
				}
			}
		}
	}

	// Then for outputs
	for(auto output : selectedDrawStruct->getObject()->getOutputDatas())
	{
		QRectF dataRect;
		if(selectedDrawStruct->getDataRect(output, dataRect))
		{
			auto dataHeight = dataRect.center().y() - y;
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
							m_snapTargetsY.insert(dataRect.center().y() - dataHeight);
					}
				}
			}
		}
	}
}

void GraphView::computeSnapDelta(ObjectDrawStruct* selectedDrawStruct, QPointF position)
{
	m_snapDelta = QPointF();
	const qreal snapMaxDist = 5;

	auto comparator = [](qreal pos) {
		return [pos](const qreal& lhs, const qreal& rhs) {
			return qAbs(pos - lhs) < qAbs(pos - rhs);
		};
	};

	// We look for the closest object above and the closest below
	const qreal filterRatio = 0.66f, filterDist = 50;
	auto selectedHeight = selectedDrawStruct->getObjectSize().height();
	auto viewRect = QRectF(contentsRect());
	auto m1 = std::numeric_limits<qreal>::lowest(), m2 = std::numeric_limits<qreal>::max();
	QPointF abovePos(m1, m1), belowPos(m2, m2);
	qreal aboveDist{ m2 }, belowDist{ m2 };
	bool hasInsideObject = false;
	std::set<qreal> snapTargetsX;
	for (const auto ods : m_orderedObjectDrawStructs)
	{
		if (ods == selectedDrawStruct || !ods->acceptsMagneticSnap())
			continue;

		auto area = ods->getObjectArea();
		if (viewRect.intersects(area)) // Only if visible in the current viewport
		{
			auto pos = ods->getPosition();
			if (pos.y() + area.height() < position.y())
			{
				// Distance from the bottom left corner of this one and the top left of the selected
				auto dist = QPointF(pos.x() - position.x(), pos.y() + area.height() - position.y()).manhattanLength();
				if (dist < aboveDist)
				{
					aboveDist = dist;
					abovePos = QPointF(pos.x() , pos.y() + area.height());
				}
			}
			else if(pos.y() > position.y() + selectedHeight)
			{
				// Distance from the top left corner of this one and the bottom left of the selected
				auto dist = QPointF(pos.x() - position.x(), pos.y() - (position.y() + selectedHeight)).manhattanLength();
				if (dist < belowDist)
				{
					belowDist = dist;
					belowPos = QPointF(pos.x() , pos.y() - selectedHeight);
				}
			}
			else if(qAbs(pos.x() - position.x()) < filterDist) // The selected one intersects the y axis of this one, and is close enough on the x axis
			{
				snapTargetsX.insert(pos.x());
				hasInsideObject = true;
			}
		}
	}
	
	if (hasInsideObject)
	{
		// Only take the other ones if their are close
		if(qAbs(abovePos.y() - position.y()) < filterDist)
			snapTargetsX.insert(abovePos.x());
		if (qAbs(belowPos.y() - position.y()) < filterDist)
			snapTargetsX.insert(belowPos.x());
	}
	else
	{
		// We only take the closest if the other one is at least 50% further
		if (aboveDist < belowDist * filterRatio && belowDist > filterDist)
			snapTargetsX.insert(abovePos.x());
		else if (belowDist < aboveDist * filterRatio && aboveDist > filterDist)
			snapTargetsX.insert(belowPos.x());
		else
		{
			snapTargetsX.insert(abovePos.x());
			snapTargetsX.insert(belowPos.x());
		}
	}

	auto minIter = std::min_element(snapTargetsX.begin(), snapTargetsX.end(), comparator(position.x()));
	if(minIter != snapTargetsX.end())
	{
		qreal x = *minIter;
		if(qAbs(x - position.x()) < snapMaxDist)
			m_snapDelta.setX(x - position.x());
	}

	minIter = std::min_element(m_snapTargetsY.begin(), m_snapTargetsY.end(), comparator(position.y()));
	if(minIter != m_snapTargetsY.end())
	{
		qreal y = *minIter;
		if(qAbs(y - position.y()) < snapMaxDist)
			m_snapDelta.setY(y - position.y());
	}
}

void GraphView::moveObjects(std::vector<panda::PandaObject*> objects, QPointF delta)
{
	for(auto object : objects)
	{
		auto ods = getObjectDrawStruct(object);
		if(ods)
			ods->move(delta);
	}
	emit modified();
	updateLinkTags();
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
		if(lpos.y() == rpos.y())
			return lpos.x() > rpos.x();
		return lpos.y() < rpos.y();
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
	for(const auto& object : m_pandaDocument->getObjects())
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
	return m_viewRect.size().toSize();
}

QPoint GraphView::viewPosition()
{
	return m_viewRect.topLeft().toPoint();
}

void GraphView::scrollView(QPoint position)
{
	QPointF delta = (position - m_viewRect.topLeft()) / m_zoomFactor;
	m_viewRect.moveTo(position);
	moveView(delta);
	update();
}

void GraphView::updateViewRect()
{
	if(m_isLoading)
		return;

	m_viewRect = QRectF();
	for(const auto& ods : m_orderedObjectDrawStructs)
	{
		QRectF area = ods->getObjectArea();
		QRectF zoomedArea = QRectF(area.topLeft() * m_zoomFactor, area.size() * m_zoomFactor);
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
		m_selectedObjectsDrawStructs.push_back(getObjectDrawStruct(object));

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
	for (const auto& object : m_pandaDocument->getObjects())
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
	m_recomputeTags = true;
	m_highlightConnectedDatas = false;
	m_hoverData = nullptr;
	m_hoverTimer->stop();
	updateViewRect();
}

void GraphView::objectsReordered()
{
	m_orderedObjectDrawStructs.clear();
	for (const auto& obj : m_pandaDocument->getObjects())
		m_orderedObjectDrawStructs.push_back(getObjectDrawStruct(obj.get()));
}
