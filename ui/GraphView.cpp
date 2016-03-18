#include <QtWidgets>
#include <cmath>
#include <functional>

#include <ui/ChooseWidgetDialog.h>
#include <ui/GraphView.h>
#include <ui/LinkTag.h>
#include <ui/QuickCreateDialog.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/drawstruct/DockableDrawStruct.h>
#include <ui/graph/ObjectsSelection.h>

#include <panda/helper/algorithm.h>
#include <panda/types/DataTraits.h>
#include <panda/command/DockableCommand.h>
#include <panda/command/LinkDatasCommand.h>
#include <ui/command/MoveObjectCommand.h>

#ifdef PANDA_LOG_EVENTS
#include <ui/UpdateLoggerDialog.h>
#endif

#include <panda/PandaDocument.h>
#include <panda/document/DocumentSignals.h>

GraphView::GraphView(panda::PandaDocument* doc, QWidget* parent)
	: QWidget(parent)
	, m_pandaDocument(doc)
	, m_zoomLevel(0)
	, m_wheelTicks(0)
	, m_zoomFactor(1.0)
	, m_movingAction(MOVING_NONE)
	, m_clickedData(nullptr)
	, m_hoverData(nullptr)
	, m_contextMenuData(nullptr)
	, m_capturedDrawStruct(nullptr)
	, m_recomputeTags(false)
	, m_hoverTimer(new QTimer(this))
	, m_highlightConnectedDatas(false)
	, m_useMagneticSnap(true)
	, m_isLoading(false)
	, m_objectsSelection(std::make_unique<ObjectsSelection>(doc))
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	m_observer.get(m_pandaDocument->getSignals().modified).connect<QWidget, &QWidget::update>(this);
	m_observer.get(m_objectsSelection->selectionChanged).connect<QWidget, &QWidget::update>(this);
	m_observer.get(m_pandaDocument->getSignals().addedObject).connect<GraphView, &GraphView::addedObject>(this);
	m_observer.get(m_pandaDocument->getSignals().removedObject).connect<GraphView, &GraphView::removeObject>(this);
	m_observer.get(m_pandaDocument->getSignals().modifiedObject).connect<GraphView, &GraphView::modifiedObject>(this);
	m_observer.get(m_pandaDocument->getSignals().savingObject).connect<GraphView, &GraphView::savingObject>(this);
	m_observer.get(m_pandaDocument->getSignals().loadingObject).connect<GraphView, &GraphView::loadingObject>(this);
	m_observer.get(m_pandaDocument->getSignals().startLoading).connect<GraphView, &GraphView::startLoading>(this);
	m_observer.get(m_pandaDocument->getSignals().loadingFinished).connect<GraphView, &GraphView::loadingFinished>(this);
	m_observer.get(m_pandaDocument->getSignals().changedDock).connect<GraphView, &GraphView::changedDock>(this);

	connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverDataInfo()));

	m_hoverTimer->setSingleShot(true);

	setMouseTracking(true);
}

GraphView::~GraphView() = default;

void GraphView::resetView()
{
	m_viewDelta = QPointF();
	m_zoomLevel = 0;
	m_zoomFactor = 1.0;
	m_objectDrawStructs.clear();
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

QPointF GraphView::getViewDelta() const
{
	return m_viewDelta;
}

panda::PandaDocument* GraphView::getDocument() const
{
	return m_pandaDocument;
}

const panda::BaseData* GraphView::getClickedData() const
{
	return m_clickedData;
}

const panda::BaseData* GraphView::getContextMenuData() const
{
	return m_contextMenuData;
}

panda::PandaObject* GraphView::getObjectAtPos(const QPointF& pt)
{
	const auto& objects = m_pandaDocument->getObjects();
	for(int i=objects.size()-1; i>=0; --i)
	{
		auto object = objects[i];
		if(m_objectDrawStructs[object.get()]->contains(pt))
			return object.get();
	}
	return nullptr;
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

std::shared_ptr<ObjectDrawStruct> GraphView::getSharedObjectDrawStruct(panda::PandaObject* object)
{
	return panda::helper::valueOrDefault(m_objectDrawStructs, object);
}

void GraphView::setObjectDrawStruct(panda::PandaObject* object, std::shared_ptr<ObjectDrawStruct> drawStruct)
{
	m_objectDrawStructs[object] = drawStruct;
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
		for(auto& drawStruct : m_objectDrawStructs)
			drawStruct.second->moveVisual(delta);

		for(auto& tag : m_linkTags)
			tag.second->moveView(delta);
	}
}

void GraphView::paintEvent(QPaintEvent* /* event */)
{
	if(m_recomputeTags)
	{
		updateLinkTags();
		m_recomputeTags = false;
	}

	QStylePainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);

	// Zoom
	painter.scale(m_zoomFactor, m_zoomFactor);

	// Give a possibility to draw behind normal objects
	for (auto& object : m_pandaDocument->getObjects())
		m_objectDrawStructs[object.get()]->drawBackground(&painter);

	// Draw links
	painter.setBrush(Qt::NoBrush);
	for (auto& object : m_pandaDocument->getObjects())
		m_objectDrawStructs[object.get()]->drawLinks(&painter);

	// Draw the objects
	for (auto& object : m_pandaDocument->getObjects())
		m_objectDrawStructs[object.get()]->draw(&painter);

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for (auto& object : m_objectsSelection->get())
		m_objectDrawStructs[object]->draw(&painter, true);

	painter.setBrush(Qt::NoBrush);
	// Give a possibility to draw in front of normal objects
	for (auto& object : m_pandaDocument->getObjects())
		m_objectDrawStructs[object.get()]->drawForeground(&painter);

	// Draw links tags
	for (auto& tag : m_linkTags)
		tag.second->draw(&painter);

	// Highlight connected Datas
	if (m_highlightConnectedDatas)
		drawConnectedDatas(&painter, m_hoverData);

	// Selection rubber band
	if (m_movingAction == MOVING_SELECTION)
	{
		QRectF selectionRect(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor);
		QPen pen(palette().text().color());
		pen.setStyle(Qt::DashDotLine);
		painter.setPen(pen);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(selectionRect);
	}

	// Link in creation
	if (m_movingAction == MOVING_LINK)
	{
		QPen pen(palette().text().color());
		pen.setStyle(Qt::DotLine);
		painter.setPen(pen);
		painter.drawLine(m_previousMousePos, m_currentMousePos);
	}

#ifdef PANDA_LOG_EVENTS
	paintLogDebug(&painter);
#endif
}

#ifdef PANDA_LOG_EVENTS
void GraphView::paintLogDebug(QPainter* painter)
{
	UpdateLoggerDialog* logDlg = UpdateLoggerDialog::getInstance();
	if(logDlg && logDlg->isVisible())
	{
		auto states = logDlg->getNodeStates();
		for(auto object : m_pandaDocument->getObjects())
		{
			auto ods = m_objectDrawStructs[object.get()];
			if(states[object.get()])
				painter->setBrush(QColor(255,0,0,32));
			else
				painter->setBrush(QColor(0,255,0,32));

			QRectF area = ods->getObjectArea();
			painter->drawRect(area);

			for(panda::BaseData* data : object->getDatas())
			{
				if(ods->getDataRect(data, area))
				{
					if(states[data])
						painter->setBrush(QColor(255,0,0,64));
					else
						painter->setBrush(QColor(0,255,0,64));

					painter->drawRect(area);
				}
			}
		}

		const panda::helper::EventData* event = logDlg->getSelectedEvent();
		if(event)
		{
			panda::PandaObject* object = m_pandaDocument->findObject(event->m_objectIndex);
			if(object)
			{
				auto ods = m_objectDrawStructs[object];
				painter->setBrush(QColor(128, 128, 255, 128));
				QRectF area;

				bool drawData = false;
				const panda::BaseData* data = dynamic_cast<const panda::BaseData*>(event->m_node);
				if(data)
					drawData = ods->getDataRect(data, area);
				if(!drawData)
					area = ods->getObjectArea();

				painter->drawRect(area);
			}
		}
	}
}
#endif

void GraphView::resizeEvent(QResizeEvent*)
{
	update();
}

void GraphView::mousePressEvent(QMouseEvent* event)
{
	QPointF zoomedMouse = event->localPos() / m_zoomFactor;
	if(event->button() == Qt::LeftButton)
	{
		panda::PandaObject* object = getObjectAtPos(zoomedMouse);
		if(object)
		{
			// Testing for Datas first
			QPointF linkStart;
			panda::BaseData* data = m_objectDrawStructs[object]->getDataAtPos(zoomedMouse, &linkStart);
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
				if(m_objectDrawStructs[object]->mousePressEvent(event))
				{
					m_movingAction = MOVING_CUSTOM;
					m_capturedDrawStruct = m_objectDrawStructs[object].get();
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
			if(m_useMagneticSnap)
			{
				auto selected = m_objectsSelection->lastSelectedObject();
				auto ods = m_objectDrawStructs[selected];
				if(ods->acceptsMagneticSnap())
				{
					prepareSnapTargets(ods.get());
					auto possiblePosition = ods->getPosition() + delta;
					computeSnapDelta(possiblePosition);
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
		if(m_useMagneticSnap)
		{
			QPointF oldSnapDelta = m_snapDelta;
			auto selected = m_objectsSelection->lastSelectedObject();
			auto possiblePosition = m_objectDrawStructs[selected]->getPosition() + delta - m_snapDelta;
			computeSnapDelta(possiblePosition);
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

		panda::PandaObject* object = getObjectAtPos(m_currentMousePos);
		if(object)
		{
			QPointF linkStart;
			panda::BaseData* data = m_objectDrawStructs[object]->getDataAtPos(m_currentMousePos, &linkStart);
			if(data && isCompatible(m_clickedData, data))
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
		panda::PandaObject* object = getObjectAtPos(zoomedMouse);
		if(object)
		{
			panda::BaseData* data = m_objectDrawStructs[object]->getDataAtPos(zoomedMouse);
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
				if(m_objectDrawStructs[object]->getDataRect(m_hoverData, dataRect))
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
					bool hover = tag->isHovering(zoomedMouse);
					if(hover != tag->hovering)
					{
						tag->hovering = hover;
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
		for(auto object : m_objectsSelection->get())
			positions[object] = m_objectDrawStructs[object]->getPosition();

		for(auto& object : m_objectsSelection->get())
		{
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			if(dockable)
			{
				QPointF delta = positions[object] - m_objectDrawStructs[dockable]->getPosition();
				m_pandaDocument->getUndoStack().push(std::make_shared<MoveObjectCommand>(this, dockable, delta));

				QRectF dockableArea = m_objectDrawStructs[dockable]->getObjectArea();
				panda::DockObject* defaultDock = dockable->getDefaultDock();
				panda::DockObject* newDock = defaultDock;
				int newIndex = -1;
				for(auto& object : m_pandaDocument->getObjects())
				{
					panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object.get());
					if(dock)
					{
						if(dockableArea.intersects(m_objectDrawStructs[dock]->getObjectArea()) && dock->accepts(dockable))
						{
							newIndex = dynamic_cast<DockObjectDrawStruct*>(m_objectDrawStructs[dock].get())->getDockableIndex(dockableArea);
							newDock = dock;
							break;
						}
					}
				}

				panda::DockObject* prevDock = dockable->getParentDock();
				if(newDock != prevDock) // Changing dock
				{
					if(prevDock)
						m_pandaDocument->getUndoStack().push(std::make_shared<DetachDockableCommand>(prevDock, dockable));
					if(newDock)
					{
						m_pandaDocument->getUndoStack().push(std::make_shared<AttachDockableCommand>(newDock, dockable, newIndex));
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

						m_pandaDocument->getUndoStack().push(std::make_shared<ReorderDockableCommand>(prevDock, dockable, newIndex));
					}
					modifiedObject(prevDock);	// Always update
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
		for(auto& object : m_pandaDocument->getObjects())
		{
			QRectF objectArea = m_objectDrawStructs[object.get()]->getObjectArea();
			if(selectionRect.contains(objectArea) || selectionRect.intersects(objectArea))
				m_objectsSelection->add(object.get());
		}

		update();
	}
	else if(m_movingAction == MOVING_LINK)
	{
		panda::PandaObject* obj = getObjectAtPos(m_currentMousePos);
		if(obj)
		{
			panda::BaseData* secondData = m_objectDrawStructs[obj]->getDataAtPos(m_currentMousePos);
			if(secondData && isCompatible(m_clickedData, secondData))
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
	panda::PandaObject* object = getObjectAtPos(zoomedMouse);
	if(object)
	{
		flags |= MENU_OBJECT;
		m_contextMenuData = m_objectDrawStructs[object]->getDataAtPos(zoomedMouse);
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
		for(auto& object : m_pandaDocument->getObjects())
		{
			QRectF objectArea = m_objectDrawStructs[object.get()]->getObjectArea();
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
		for(auto& object : m_pandaDocument->getObjects())
		{
			QRectF objectArea = m_objectDrawStructs[object.get()]->getObjectArea();
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
		for(auto object : m_objectsSelection->get())
		{
			QRectF objectArea = m_objectDrawStructs[object]->getObjectArea();
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
		for(auto object : m_objectsSelection->get())
		{
			QRectF objectArea = m_objectDrawStructs[object]->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		QPointF delta = contentsRect().center() / m_zoomFactor - totalView.center();

		for(auto object : m_objectsSelection->get())
		{
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			// Do not move (docked) dockable objects, their parent dock move them already
			if(!dockable || !m_objectsSelection->isSelected(dockable->getParentDock()))
				m_objectDrawStructs[object]->move(delta);
		}

		update();
		updateViewRect();
	}
}

void GraphView::addedObject(panda::PandaObject* object)
{
	// Creating a DrawStruct depending on the class of the object been added
	// When undoing a delete command, the DrawStruct has already been reinserted
	if(!m_objectDrawStructs.count(object))
	{
		ObjectDrawStruct* ods = ObjectDrawStructFactory::getInstance()->createDrawStruct(this, object);
		m_objectDrawStructs.emplace(object, std::shared_ptr<ObjectDrawStruct>(ods));
	}

	update();
	updateViewRect();
}

void GraphView::removeObject(panda::PandaObject* object)
{
	m_objectDrawStructs.erase(object);
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
	if(m_objectDrawStructs.count(object))	// Can be called before the object is fully created
	{
		m_objectDrawStructs[object]->update();
		m_linkTags.clear();
		m_recomputeTags = true;
		m_highlightConnectedDatas = false;
		m_hoverData = nullptr;
		m_hoverTimer->stop();
		update();
		updateViewRect(); // The size of the object can have changed
	}
}

void GraphView::savingObject(panda::XmlElement& elem, panda::PandaObject* object)
{
	m_objectDrawStructs[object]->save(elem);
}

void GraphView::loadingObject(const panda::XmlElement& elem, panda::PandaObject* object)
{
	m_objectDrawStructs[object]->load(elem);
}

int GraphView::getAvailableLinkTagIndex()
{
	int nb = m_linkTags.size();
	QVector<bool> indices(nb, true);

	for(auto& tagPair : m_linkTags)
	{
		auto& tag = tagPair.second;
		if(tag->index < nb)
			indices[tag->index] = false;
	}

	for(int i=0; i<nb; ++i)
	{
		if(indices[i])
			return i;
	}

	return nb;
}

qreal GraphView::getZoom()
{
	return m_zoomFactor;
}

void GraphView::addLinkTag(panda::BaseData* input, panda::BaseData* output)
{
	if(m_linkTags.count(input))
		m_linkTags[input]->addOutput(output);
	else
	{
		std::shared_ptr<LinkTag> tag(new LinkTag(this, input, output));
		tag->index = getAvailableLinkTagIndex();
		m_linkTags[input] = tag;
	}
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
				if(ix >= ox)
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

void GraphView::drawConnectedDatas(QStylePainter* painter, panda::BaseData* sourceData)
{
	QVector<QRectF> highlightRects;
	QVector< QPair<QPointF, QPointF> > highlightLinks;

	QRectF sourceRect;
	if(m_objectDrawStructs[sourceData->getOwner()]->getDataRect(sourceData, sourceRect))
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
				if(m_objectDrawStructs.count(object))
				{
					QRectF rect;
					if(m_objectDrawStructs[object]->getDataRect(data, rect))
					{
						highlightRects.push_back(rect);
						highlightLinks.push_back(qMakePair(rect.center(), sourceRect.center()));
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
			if(m_objectDrawStructs.count(object))
			{
				QRectF rect;
				if(m_objectDrawStructs[object]->getDataRect(data, rect))
				{
					highlightRects.push_back(rect);
					highlightLinks.push_back(qMakePair(sourceRect.center(), rect.center()));
				}
			}
		}
	}

	if(highlightLinks.empty())
		return;

	// Now draw everything
	painter->setBrush(palette().highlight().color());
	painter->setPen(palette().text().color());
	for(const auto& rect : highlightRects)
		painter->drawRect(rect);

	painter->setPen(QPen(palette().highlight(), 3));
	painter->setBrush(Qt::NoBrush);
	for(const auto& link : highlightLinks)
	{
		if(link.first.x()-link.second.x() > 0) // We don't draw a link if it goes from right to left (see the LinkTag class)
		{
			double w = (link.second.x()-link.first.x()) / 2;
			QPainterPath path;
			path.moveTo(link.first);
			path.cubicTo(link.first+QPointF(w,0), link.second-QPointF(w,0), link.second);
			painter->drawPath(path);
		}
	}
}

void GraphView::prepareSnapTargets(ObjectDrawStruct* selectedDrawStruct)
{
	m_snapTargetsX.clear();
	m_snapTargetsY.clear();

	auto viewRect = QRectF(contentsRect());

	// Use x position of every visible object
	for(const auto& odsPair : m_objectDrawStructs)
	{
		const auto& ods = odsPair.second;
		if(ods.get() == selectedDrawStruct || !ods->acceptsMagneticSnap())
			continue;

		if(viewRect.intersects(ods->getObjectArea()))
		{
			auto pos = ods->getPosition();
			m_snapTargetsX.insert(pos.x());
		}
	}

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
					if(m_objectDrawStructs.count(owner))
					{
						auto ods = m_objectDrawStructs[owner];
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
					if(m_objectDrawStructs.count(owner))
					{
						auto ods = m_objectDrawStructs[owner];
						if(ods->getDataRect(data2, dataRect))
							m_snapTargetsY.insert(dataRect.center().y() - dataHeight);
					}
				}
			}
		}
	}
}

void GraphView::computeSnapDelta(QPointF position)
{
	m_snapDelta = QPointF();
	const qreal snapMaxDist = 5;

	// Give me c++14 lambdas !
	auto comparator = [](qreal pos) -> std::function<bool(const qreal& lhs, const qreal& rhs)>	{
		return [pos](const qreal& lhs, const qreal& rhs) {
			return qAbs(pos - lhs) < qAbs(pos - rhs);
		};
	};

	auto minIter = std::min_element(m_snapTargetsX.begin(), m_snapTargetsX.end(), comparator(position.x()));
	if(minIter != m_snapTargetsX.end())
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
	m_pandaDocument->getUndoStack().push(std::make_shared<LinkDatasCommand>(target, parent));
}

void GraphView::setRecomputeTags()
{
	m_recomputeTags = true;
}

void GraphView::sortDockable(panda::DockableObject* dockable, panda::DockObject* defaultDock)
{
	int prevIndex = defaultDock->getIndexOfDockable(dockable);
	auto dockables = defaultDock->getDockedObjects();

	std::sort(dockables.begin(), dockables.end(), [this](panda::DockableObject* lhs, panda::DockableObject* rhs){
		auto lpos = m_objectDrawStructs[lhs]->getPosition();
		auto rpos = m_objectDrawStructs[rhs]->getPosition();
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

		m_pandaDocument->getUndoStack().push(std::make_shared<ReorderDockableCommand>(defaultDock, dockable, newIndex));
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
	sortAllDockables();
	m_isLoading = false;
	updateViewRect();
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
	for(const auto& ods : m_objectDrawStructs)
	{
		QRectF area = ods.second->getObjectArea();
		QRectF zoomedArea = QRectF(area.topLeft() * m_zoomFactor, area.size() * m_zoomFactor);
		m_viewRect |= zoomedArea; // Union
	}

	if(!m_objectDrawStructs.empty())
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
