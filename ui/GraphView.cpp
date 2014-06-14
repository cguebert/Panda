#include <QtWidgets>
#include <cmath>
#include <functional>

#include <ui/GraphView.h>
#include <ui/LinkTag.h>
#include <ui/QuickCreateDialog.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/drawstruct/DockableDrawStruct.h>

#include <ui/command/DetachDockableCommand.h>
#include <ui/command/LinkDatasCommand.h>
#include <ui/command/MoveObjectCommand.h>

#ifdef PANDA_LOG_EVENTS
#include <ui/UpdateLoggerDialog.h>
#endif

#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>

GraphView::GraphView(panda::PandaDocument* doc, QWidget* parent)
	: QWidget(parent)
	, m_pandaDocument(doc)
	, m_zoomLevel(0)
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
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	connect(m_pandaDocument, SIGNAL(modified()), this, SLOT(update()));
	connect(m_pandaDocument, SIGNAL(selectionChanged()), this, SLOT(update()));
	connect(m_pandaDocument, SIGNAL(addedObject(panda::PandaObject*)), this, SLOT(addedObject(panda::PandaObject*)));
	connect(m_pandaDocument, SIGNAL(removedObject(panda::PandaObject*)), this, SLOT(removeObject(panda::PandaObject*)));
	connect(m_pandaDocument, SIGNAL(modifiedObject(panda::PandaObject*)), this, SLOT(modifiedObject(panda::PandaObject*)));
	connect(m_pandaDocument, SIGNAL(savingObject(QDomDocument&,QDomElement&,panda::PandaObject*)), this, SLOT(savingObject(QDomDocument&,QDomElement&,panda::PandaObject*)));
	connect(m_pandaDocument, SIGNAL(loadingObject(QDomElement&,panda::PandaObject*)), this, SLOT(loadingObject(QDomElement&,panda::PandaObject*)));
	connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverDataInfo()));

	m_hoverTimer->setSingleShot(true);

	setMouseTracking(true);
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
	auto objects = m_pandaDocument->getObjects();
	for(int i=objects.size()-1; i>=0; --i)
	{
		auto object = objects[i];
		if(m_objectDrawStructs[object.data()]->contains(pt))
			return object.data();
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
	m_recomputeTags = false;
	m_highlightConnectedDatas = false;
}

ObjectDrawStruct* GraphView::getObjectDrawStruct(panda::PandaObject* object)
{
	return m_objectDrawStructs.value(object).data();
}

QSharedPointer<ObjectDrawStruct> GraphView::getSharedObjectDrawStruct(panda::PandaObject* object)
{
	return m_objectDrawStructs.value(object);
}

void GraphView::setObjectDrawStruct(panda::PandaObject* object, QSharedPointer<ObjectDrawStruct> drawStruct)
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
			drawStruct->moveVisual(delta);

		for(auto tag : m_linkTags.values())
			tag->moveView(delta);
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
	for(auto object : m_pandaDocument->getObjects())
		m_objectDrawStructs[object.data()]->drawBackground(&painter);

	// Draw the objects
	for(auto object : m_pandaDocument->getObjects())
		m_objectDrawStructs[object.data()]->draw(&painter);

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for(auto object : m_pandaDocument->getSelection())
		m_objectDrawStructs[object]->draw(&painter);

	// Draw links
	painter.setBrush(Qt::NoBrush);
	for(auto object : m_pandaDocument->getObjects())
		m_objectDrawStructs[object.data()]->drawLinks(&painter);

	// Give a possibility to draw in front of normal objects
	for(auto object : m_pandaDocument->getObjects())
		m_objectDrawStructs[object.data()]->drawForeground(&painter);

	// Draw links tags
	for(auto& tag : m_linkTags.values())
		tag->draw(&painter);

	// Highlight connected Datas
	if(m_highlightConnectedDatas)
		drawConnectedDatas(&painter, m_hoverData);

	// Selection rubber band
	if(m_movingAction == MOVING_SELECTION)
	{
		QRectF selectionRect(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor);
		QPen pen(palette().text().color());
		pen.setStyle(Qt::DashDotLine);
		painter.setPen(pen);
		painter.drawRect(selectionRect);
	}

	// Link in creation
	if(m_movingAction == MOVING_LINK)
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
			auto ods = m_objectDrawStructs[object.data()];
			if(states[object.data()])
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

						m_pandaDocument->selectNone();
					}
				}
			}
			else	// No Data, but we still clicked on an object
			{
				// Add the object to the selection
				if(event->modifiers() == Qt::ControlModifier)
				{
					if(m_pandaDocument->isSelected(object))
						m_pandaDocument->selectionRemove(object);
					else
						m_pandaDocument->selectionAdd(object);
				}
				else	// Moving the object (or selecting only this one if we release the mouse without moving)
				{
					if(!m_pandaDocument->isSelected(object))
						m_pandaDocument->selectNone();

					m_pandaDocument->setCurrentSelectedObject(object);
					m_movingAction = MOVING_START;
					m_previousMousePos = zoomedMouse;
				}

				// Maybe do a custom action ?
				if(m_objectDrawStructs[object]->mousePressEvent(event))
				{
					m_movingAction = MOVING_CUSTOM;
					m_capturedDrawStruct = m_objectDrawStructs[object].data();
				}
			}
		}
		else
		{
			// Clicked where there is nothing
			// Starting a rubber band to select in a zone
			m_pandaDocument->selectNone();
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
				auto selected = m_pandaDocument->getCurrentSelectedObject();
				auto ods = m_objectDrawStructs[selected];
				if(ods->acceptsMagneticSnap())
				{
					prepareSnapTargets(ods.data());
					auto possiblePosition = ods->getPosition() + delta;
					computeSnapDelta(possiblePosition);
					delta = delta + m_snapDelta;
				}
			}
			QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));

			// Remove docked objects from the selection
			m_customSelection.clear();
			for(auto object : m_pandaDocument->getSelection())
			{
				panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
				if(dockable && m_pandaDocument->isSelected(dockable->getParentDock()))
					continue; // don't move a dockable object if their parent dock is selected, it will move them
				m_customSelection.push_back(object);
			}

			m_moveObjectsMacro = m_pandaDocument->beginCommandMacro(tr("move objects"));

			if(!delta.isNull())
				m_pandaDocument->addCommand(new MoveObjectCommand(this, m_customSelection, delta));

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
			auto selected = m_pandaDocument->getCurrentSelectedObject();
			auto possiblePosition = m_objectDrawStructs[selected]->getPosition() + delta - m_snapDelta;
			computeSnapDelta(possiblePosition);
			delta = delta - oldSnapDelta + m_snapDelta;
		}

		if(!m_customSelection.empty() && !delta.isNull())
			m_pandaDocument->addCommand(new MoveObjectCommand(this, m_customSelection, delta));

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
					m_hoverTimer->start(1000);
			}

			if(m_hoverData)
			{
				QRectF dataRect;
				if(m_objectDrawStructs[object]->getDataRect(m_hoverData, dataRect))
				{
					QString display = QString("%1\n%2")
							.arg(m_hoverData->getName())
							.arg(m_hoverData->getDescription());
					QToolTip::showText(event->globalPos(), display, this, dataRect.toRect());
					if(!m_hoverData->getHelp().isEmpty())
						emit showStatusBarMessage(m_hoverData->getHelp());
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
				for(auto& tag : m_linkTags)
				{
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
		panda::PandaObject* object = m_pandaDocument->getCurrentSelectedObject();
		if(object)
		{
			m_pandaDocument->selectNone();
			m_pandaDocument->selectionAdd(object);
		}
	}
	else if(m_movingAction == MOVING_OBJECT)
	{
		QMap<panda::PandaObject*, QPointF> positions;
		for(auto object : m_pandaDocument->getSelection())
			positions[object] = m_objectDrawStructs[object]->getPosition();

		for(auto object : m_pandaDocument->getSelection())
		{
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			if(dockable)
			{
				QPointF delta = positions[object] - m_objectDrawStructs[dockable]->getPosition();
				m_pandaDocument->addCommand(new MoveObjectCommand(this, dockable, delta));

				QRectF dockableArea = m_objectDrawStructs[dockable]->getObjectArea();
				panda::DockObject* defaultDock = dockable->getDefaultDock();
				panda::DockObject* newDock = defaultDock;
				int newIndex = -1;
				for(auto object : m_pandaDocument->getObjects())
				{
					panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object.data());
					if(dock)
					{
						if(dockableArea.intersects(m_objectDrawStructs[dock]->getObjectArea()) && dock->accepts(dockable))
						{
							newIndex = dynamic_cast<DockObjectDrawStruct*>(m_objectDrawStructs[dock].data())->getDockableIndex(dockableArea);
							newDock = dock;
							break;
						}
					}
				}

				panda::DockObject* prevDock = dockable->getParentDock();
				if(newDock != prevDock)
				{
					if(prevDock)
						m_pandaDocument->addCommand(new DetachDockableCommand(prevDock, dockable));
					if(newDock)
						m_pandaDocument->addCommand(new AttachDockableCommand(newDock, dockable, newIndex));
				}
				else if(prevDock != defaultDock)
				{
					int prevIndex = prevDock->getIndexOfDockable(dockable);
					if(prevIndex != newIndex)
					{
						if(newIndex > prevIndex)
							--newIndex;

						m_pandaDocument->addCommand(new DetachDockableCommand(prevDock, dockable));
						m_pandaDocument->addCommand(new AttachDockableCommand(prevDock, dockable, newIndex));
					}
					modifiedObject(prevDock);	// Always update
				}
			}
		}

		m_moveObjectsMacro.reset();
	}
	else if(m_movingAction == MOVING_SELECTION)
	{
		m_pandaDocument->selectNone();

		QRectF selectionRect = QRectF(m_previousMousePos/m_zoomFactor, m_currentMousePos/m_zoomFactor).normalized();
		for(auto object : m_pandaDocument->getObjects())
		{
			QRectF objectArea = m_objectDrawStructs[object.data()]->getObjectArea();
			if(selectionRect.contains(objectArea) || selectionRect.intersects(objectArea))
				m_pandaDocument->selectionAdd(object.data());
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
		}
	}

	QApplication::restoreOverrideCursor();
	m_movingAction = MOVING_NONE;
}

void GraphView::wheelEvent(QWheelEvent * event)
{
	if(m_movingAction != MOVING_NONE)
	{
		event->ignore();
		return;
	}

	int ticks = event->delta() / 40;
	int newZoom = qBound(0, m_zoomLevel-ticks, 90);
	if(m_zoomLevel != newZoom)
	{
		QPointF mousePos = event->pos();
		QPointF oldPos = mousePos / m_zoomFactor;
		m_zoomLevel = newZoom;
		m_zoomFactor = (100 - m_zoomLevel) / 100.0;
		moveView(mousePos / m_zoomFactor - oldPos);
		update();
	}
}

void GraphView::keyPressEvent(QKeyEvent * event)
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
			update();
		}
		break;
	case Qt::Key_Right:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(QPointF(-100, 0));
			update();
		}
		break;
	case Qt::Key_Up:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(QPointF(0, 100));
			update();
		}
		break;
	case Qt::Key_Down:
		if(event->modifiers() & Qt::ControlModifier)
		{
			moveView(QPointF(0, -100));
			update();
		}
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
		panda::BaseData* data = m_objectDrawStructs[object]->getDataAtPos(zoomedMouse);
		if(data)
		{
			flags |= MENU_DATA;
			m_contextMenuData = data;
			if(m_contextMenuData->isInput() && m_contextMenuData->getParent())
				flags |= MENU_LINK;
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
	}
}

void GraphView::centerView()
{
	if(m_pandaDocument->getNbObjects())
	{
		QRectF totalView;
		for(auto object : m_pandaDocument->getObjects())
		{
			QRectF objectArea = m_objectDrawStructs[object.data()]->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		moveView(contentsRect().center() / m_zoomFactor - totalView.center());
		update();
	}
}

void GraphView::showAll()
{
	if(m_pandaDocument->getNbObjects())
	{
		QRectF totalView;
		for(auto object : m_pandaDocument->getObjects())
		{
			QRectF objectArea = m_objectDrawStructs[object.data()]->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		qreal factorW = contentsRect().width() / (totalView.width() + 40);
		qreal factorH = contentsRect().height() / (totalView.height() + 40);
		m_zoomFactor = qBound(0.1, qMin(factorW, factorH), 1.0);
		m_zoomLevel = 100 * (1.0 - m_zoomFactor);
		moveView(contentsRect().center() / m_zoomFactor - totalView.center());
		update();
	}
}

void GraphView::showAllSelected()
{
	if(m_pandaDocument->getNbSelected())
	{
		QRectF totalView;
		for(auto object : m_pandaDocument->getSelection())
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
	}
}

void GraphView::moveSelectedToCenter()
{
	if(m_pandaDocument->getNbSelected())
	{
		QRectF totalView;
		for(auto object : m_pandaDocument->getSelection())
		{
			QRectF objectArea = m_objectDrawStructs[object]->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		QPointF delta = contentsRect().center() / m_zoomFactor - totalView.center();

		for(auto object : m_pandaDocument->getSelection())
		{
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			// Do not move (docked) dockable objects, their parent dock move them already
			if(!dockable || !m_pandaDocument->isSelected(dockable->getParentDock()))
				m_objectDrawStructs[object]->move(delta);
		}

		update();
	}
}

void GraphView::addedObject(panda::PandaObject* object)
{
	// Creating a DrawStruct depending on the class of the object been added
	// When undoing a delete command, the DrawStruct has already been reinserted
	if(!m_objectDrawStructs.contains(object))
	{
		ObjectDrawStruct* ods = ObjectDrawStructFactory::getInstance()->createDrawStruct(this, object);
		m_objectDrawStructs.insert(object, QSharedPointer<ObjectDrawStruct>(ods));
	}

	update();
}

void GraphView::removeObject(panda::PandaObject* object)
{
	m_objectDrawStructs.remove(object);
	m_capturedDrawStruct = nullptr;
	m_movingAction = MOVING_NONE;
	m_linkTags.clear();
	m_recomputeTags = true;
	m_highlightConnectedDatas = false;
	update();
}

void GraphView::modifiedObject(panda::PandaObject* object)
{
	if(m_objectDrawStructs.contains(object))	// Can be called before the object is fully created
	{
		m_objectDrawStructs[object]->update();
		m_linkTags.clear();
		m_recomputeTags = true;
		m_highlightConnectedDatas = false;
		m_hoverData = nullptr;
		m_hoverTimer->stop();
		update();
	}
}

void GraphView::savingObject(QDomDocument& doc, QDomElement& elem, panda::PandaObject* object)
{
	m_objectDrawStructs[object]->save(doc, elem);
}

void GraphView::loadingObject(QDomElement& elem, panda::PandaObject* object)
{
	m_objectDrawStructs[object]->load(elem);
}

int GraphView::getAvailableLinkTagIndex()
{
	int nb = m_linkTags.size();
	QVector<bool> indices(nb, true);

	for(auto& tag : m_linkTags)
	{
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
	if(m_linkTags.contains(input))
		m_linkTags[input]->addOutput(output);
	else
	{
		QSharedPointer<LinkTag> tag(new LinkTag(this, input, output));
		tag->index = getAvailableLinkTagIndex();
		m_linkTags[input] = tag;
	}
}

void GraphView::removeLinkTag(panda::BaseData* input, panda::BaseData* output)
{
	if(m_linkTags.contains(input))
	{
		LinkTag* tag = m_linkTags[input].data();
		tag->removeOutput(output);
		if(tag->isEmpty())
			m_linkTags.remove(input);
	}
}

void GraphView::updateLinkTags(bool reset)
{
	if(reset)
		m_linkTags.clear();

	for(auto object : m_pandaDocument->getObjects())
	{
		for(auto data : object->getInputDatas())
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

	QMutableMapIterator<panda::BaseData*, QSharedPointer<LinkTag> > iterTag(m_linkTags);
	while(iterTag.hasNext())
	{
		iterTag.next();
		iterTag.value()->update();
		if(iterTag.value()->isEmpty())
			iterTag.remove();
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
				if(m_objectDrawStructs.contains(object))
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
			if(m_objectDrawStructs.contains(object))
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
	for(auto ods : m_objectDrawStructs)
	{
		if(ods == selectedDrawStruct || !ods->acceptsMagneticSnap())
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
					if(m_objectDrawStructs.contains(owner))
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
					if(m_objectDrawStructs.contains(owner))
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

void GraphView::moveObjects(QList<panda::PandaObject*> objects, QPointF delta)
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
	auto macro = m_pandaDocument->beginCommandMacro(tr("change link"));
	m_pandaDocument->addCommand(new LinkDatasCommand(target, parent));
}

void GraphView::setRecomputeTags()
{
	m_recomputeTags = true;
}
