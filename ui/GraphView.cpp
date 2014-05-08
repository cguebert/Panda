#include <QtWidgets>
#include <cmath>
#include <functional>

#include <ui/GraphView.h>
#include <ui/QuickCreateDialog.h>
#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/drawstruct/DockableDrawStruct.h>

#ifdef PANDA_LOG_EVENTS
#include <ui/UpdateLoggerDialog.h>
#endif

#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>

GraphView::GraphView(panda::PandaDocument* doc, QWidget *parent)
	: QWidget(parent)
	, pandaDocument(doc)
	, zoomLevel(0)
	, zoomFactor(1.0)
	, movingAction(MOVING_NONE)
	, clickedData(nullptr)
	, hoverData(nullptr)
	, contextMenuData(nullptr)
	, capturedDrawStruct(nullptr)
	, recomputeTags(false)
	, hoverTimer(new QTimer(this))
	, highlightConnectedDatas(false)
	, useMagneticSnap(true)
{
	setAutoFillBackground(true);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	connect(pandaDocument, SIGNAL(modified()), this, SLOT(update()));
	connect(pandaDocument, SIGNAL(selectionChanged()), this, SLOT(update()));
	connect(pandaDocument, SIGNAL(addedObject(panda::PandaObject*)), this, SLOT(addedObject(panda::PandaObject*)));
	connect(pandaDocument, SIGNAL(removedObject(panda::PandaObject*)), this, SLOT(removeObject(panda::PandaObject*)));
	connect(pandaDocument, SIGNAL(modifiedObject(panda::PandaObject*)), this, SLOT(modifiedObject(panda::PandaObject*)));
	connect(pandaDocument, SIGNAL(savingObject(QDomDocument&,QDomElement&,panda::PandaObject*)), this, SLOT(savingObject(QDomDocument&,QDomElement&,panda::PandaObject*)));
	connect(pandaDocument, SIGNAL(loadingObject(QDomElement&,panda::PandaObject*)), this, SLOT(loadingObject(QDomElement&,panda::PandaObject*)));
	connect(hoverTimer, SIGNAL(timeout()), this, SLOT(hoverDataInfo()));

	hoverTimer->setSingleShot(true);

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
	return viewDelta;
}

const panda::PandaDocument* GraphView::getDocument() const
{
	return pandaDocument;
}

const panda::BaseData* GraphView::getClickedData() const
{
	return clickedData;
}

const panda::BaseData* GraphView::getContextMenuData() const
{
	return contextMenuData;
}

panda::PandaObject* GraphView::getObjectAtPos(const QPointF& pt)
{
	auto objects = pandaDocument->getObjects();
	for(int i=objects.size()-1; i>=0; --i)
	{
		auto object = objects[i];
		if(objectDrawStructs[object]->contains(pt))
			return object;
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
	viewDelta = QPointF();
	zoomLevel = 0;
	zoomFactor = 1.0;
	objectDrawStructs.clear();
	linkTags.clear();
	movingAction = MOVING_NONE;
	clickedData = nullptr;
	hoverData = nullptr;
	contextMenuData = nullptr;
	recomputeTags = false;
	highlightConnectedDatas = false;
}

ObjectDrawStruct* GraphView::getObjectDrawStruct(panda::PandaObject* obj)
{
	return objectDrawStructs[obj].data();
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
		viewDelta += delta;
		for(auto& drawStruct : objectDrawStructs)
			drawStruct->moveVisual(delta);

		for(auto tag : linkTags.values())
			tag->moveView(delta);
	}
}

void GraphView::paintEvent(QPaintEvent* /* event */)
{
	if(recomputeTags)
	{
		updateLinkTags();
		recomputeTags = false;
	}

	QStylePainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);

	// Zoom
	painter.scale(zoomFactor, zoomFactor);

	// Give a possibility to draw behind normal objects
	for(auto object : pandaDocument->getObjects())
		objectDrawStructs[object]->drawBackground(&painter);

	// Draw the objects
	for(auto object : pandaDocument->getObjects())
		objectDrawStructs[object]->draw(&painter);

	// Redraw selected objets in case they are moved over others (so that they don't appear under them)
	for(auto object : pandaDocument->getSelection())
		objectDrawStructs[object]->draw(&painter);

	// Draw links
	painter.setBrush(Qt::NoBrush);
	for(auto object : pandaDocument->getObjects())
		objectDrawStructs[object]->drawLinks(&painter);

	// Give a possibility to draw in front of normal objects
	for(auto object : pandaDocument->getObjects())
		objectDrawStructs[object]->drawForeground(&painter);

	// Draw links tags
	for(auto& tag : linkTags.values())
		tag->draw(&painter);

	// Highlight connected Datas
	if(highlightConnectedDatas)
		drawConnectedDatas(&painter, hoverData);

	// Selection rubber band
	if(movingAction == MOVING_SELECTION)
	{
		QRectF selectionRect(previousMousePos/zoomFactor, currentMousePos/zoomFactor);
		QPen pen(palette().text().color());
		pen.setStyle(Qt::DashDotLine);
		painter.setPen(pen);
		painter.drawRect(selectionRect);
	}

	// Link in creation
	if(movingAction == MOVING_LINK)
	{
		QPen pen(palette().text().color());
		pen.setStyle(Qt::DotLine);
		painter.setPen(pen);
		painter.drawLine(previousMousePos, currentMousePos);
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
		for(panda::PandaObject* object : pandaDocument->getObjects())
		{
			auto ods = objectDrawStructs[object];
			if(states[object])
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
			panda::PandaObject* object = pandaDocument->findObject(event->m_objectIndex);
			if(object)
			{
				auto ods = objectDrawStructs[object];
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
	QPointF zoomedMouse = event->localPos() / zoomFactor;
	if(event->button() == Qt::LeftButton)
	{
		panda::PandaObject* object = getObjectAtPos(zoomedMouse);
		if(object)
		{
			// Testing for Datas first
			QPointF linkStart;
			panda::BaseData* data = objectDrawStructs[object]->getDataAtPos(zoomedMouse, &linkStart);
			if(data)
			{
				if(!pandaDocument->animationIsPlaying())
				{
					// Remove
					if(data->isInput() && data->getParent() && event->modifiers() == Qt::ControlModifier)
					{
						removeLinkTag(data->getParent(), data);
						object->dataSetParent(data, nullptr);
					}
					else	// Creating a new Link
					{
						clickedData = data;
						movingAction = MOVING_LINK;
						previousMousePos = currentMousePos = linkStart;

						pandaDocument->selectNone();
					}
				}
			}
			else	// No Data, but we still clicked on an object
			{
				// Add the object to the selection
				if(event->modifiers() == Qt::ControlModifier)
				{
					if(pandaDocument->isSelected(object))
						pandaDocument->selectionRemove(object);
					else
						pandaDocument->selectionAdd(object);
				}
				else	// Moving the object (or selecting only this one if we release the mouse without moving)
				{
					if(!pandaDocument->isSelected(object))
						pandaDocument->selectNone();

					pandaDocument->setCurrentSelectedObject(object);
					movingAction = MOVING_START;
					previousMousePos = zoomedMouse;
				}

				// Maybe do a custom action ?
				if(objectDrawStructs[object]->mousePressEvent(event))
				{
					movingAction = MOVING_CUSTOM;
					capturedDrawStruct = objectDrawStructs[object].data();
				}
			}
		}
		else
		{
			// Clicked where there is nothing
			// Starting a rubber band to select in a zone
			pandaDocument->selectNone();
			movingAction = MOVING_SELECTION;
			previousMousePos = currentMousePos = event->localPos();
			QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
		}
	}
	else if(event->button() == Qt::MidButton)
	{
		if(event->modifiers() == Qt::ControlModifier)
		{
			movingAction = MOVING_ZOOM;
			currentMousePos = event->pos();
			previousMousePos = event->globalPos();

			QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
		}
		else
		{
			movingAction = MOVING_VIEW;
			previousMousePos = event->globalPos();

			QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
		}
	}
}

void GraphView::mouseMoveEvent(QMouseEvent* event)
{
	if(movingAction == MOVING_START)
	{
		QPointF mousePos = event->localPos() / zoomFactor;
		QPointF delta = mousePos - previousMousePos;
		if(QVector2D(delta *  zoomFactor).length() > 5)
		{
			movingAction = MOVING_OBJECT;
			if(useMagneticSnap)
			{
				auto selected = pandaDocument->getCurrentSelectedObject();
				auto ods = objectDrawStructs[selected];
				if(ods->acceptsMagneticSnap())
				{
					prepareSnapTargets(ods.data());
					auto possiblePosition = ods->getPosition() + delta;
					computeSnapDelta(possiblePosition);
					delta = delta + snapDelta;
				}
			}
			QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
			if(!delta.isNull())
			{
				for(auto object : pandaDocument->getSelection())
				{
					panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
					if(dockable && pandaDocument->isSelected(dockable->getParentDock()))
						continue; // don't move a dockable object if their parent dock is selected, it will move them
					objectDrawStructs[object]->move(delta);
				}
				emit modified();
				updateLinkTags();
			}
			previousMousePos = mousePos;
			update();
		}
	}
	else if(movingAction == MOVING_OBJECT)
	{
		QPointF mousePos = event->localPos() / zoomFactor;
		QPointF delta = mousePos - previousMousePos;
		if(useMagneticSnap)
		{
			QPointF oldSnapDelta = snapDelta;
			auto selected = pandaDocument->getCurrentSelectedObject();
			auto possiblePosition = objectDrawStructs[selected]->getPosition() + delta - snapDelta;
			computeSnapDelta(possiblePosition);
			delta = delta - oldSnapDelta + snapDelta;
		}
		if(!delta.isNull())
		{
			for(auto object : pandaDocument->getSelection())
			{
				panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
				if(dockable && pandaDocument->isSelected(dockable->getParentDock()))
					continue; // don't move a dockable object if their parent dock is selected, it will move them
				objectDrawStructs[object]->move(delta);
			}
			emit modified();
			updateLinkTags();
		}
		previousMousePos = mousePos;
		update();
	}
	else if(movingAction == MOVING_VIEW)
	{
		QPointF delta = (event->globalPos() - previousMousePos) / zoomFactor;
		moveView(delta);
		previousMousePos = event->globalPos();
		update();
	}
	else if(movingAction == MOVING_ZOOM)
	{
		int y = event->globalY() - previousMousePos.y();
		QPointF oldPos = currentMousePos / zoomFactor;
		zoomFactor = qBound(0.1, zoomFactor - y / 500.0, 1.0);
		zoomLevel = 100.0 * (1.0 - zoomFactor);
		moveView(currentMousePos / zoomFactor - oldPos);
		previousMousePos = event->globalPos();
		update();
	}
	else if(movingAction == MOVING_SELECTION)
	{
		currentMousePos = event->localPos();
		update();
	}
	else if(movingAction == MOVING_LINK)
	{
		currentMousePos = event->localPos() / zoomFactor;

		panda::PandaObject* object = getObjectAtPos(currentMousePos);
		if(object)
		{
			QPointF linkStart;
			panda::BaseData* data = objectDrawStructs[object]->getDataAtPos(currentMousePos, &linkStart);
			if(data && isCompatible(clickedData, data))
				currentMousePos = linkStart;
		}
		update();
	}
	else if(movingAction == MOVING_CUSTOM)
	{
		if(capturedDrawStruct)
			capturedDrawStruct->mouseMoveEvent(event);
	}

	if(movingAction == MOVING_NONE || movingAction == MOVING_LINK)
	{
		QPointF zoomedMouse = event->localPos() / zoomFactor;
		panda::PandaObject* object = getObjectAtPos(zoomedMouse);
		if(object)
		{
			panda::BaseData* data = objectDrawStructs[object]->getDataAtPos(zoomedMouse);
			if(hoverData != data)
			{
				hoverTimer->stop();
				if(highlightConnectedDatas)
				{
					highlightConnectedDatas = false;
					update();
				}
				hoverData = data;
				if(hoverData)
					hoverTimer->start(1000);
			}

			if(hoverData)
			{
				QRectF dataRect;
				if(objectDrawStructs[object]->getDataRect(hoverData, dataRect))
				{
					QString display = QString("%1\n%2")
							.arg(hoverData->getName())
							.arg(hoverData->getDescription());
					QToolTip::showText(event->globalPos(), display, this, dataRect.toRect());
					if(!hoverData->getHelp().isEmpty())
						emit showStatusBarMessage(hoverData->getHelp());
				}
			}
		}
		else
		{
			hoverData = nullptr;
			if(highlightConnectedDatas)
			{
				highlightConnectedDatas = false;
				update();
			}
			if(hoverTimer->isActive())
				hoverTimer->stop();

			if(movingAction == MOVING_NONE)
			{
				// Look for link tags
				for(auto& tag : linkTags)
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
	if(movingAction == MOVING_START)
	{
		panda::PandaObject* object = pandaDocument->getCurrentSelectedObject();
		if(object)
		{
			pandaDocument->selectNone();
			pandaDocument->selectionAdd(object);
		}
	}
	else if(movingAction == MOVING_OBJECT)
	{
		QMap<panda::PandaObject*, QPointF> positions;
		for(auto object : pandaDocument->getSelection())
			positions[object] = objectDrawStructs[object]->getPosition();

		for(auto object : pandaDocument->getSelection())
		{
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			if(dockable)
			{
				objectDrawStructs[dockable]->move(positions[object] - objectDrawStructs[dockable]->getPosition());
				QRectF dockableArea = objectDrawStructs[dockable]->getObjectArea();
				panda::DockObject* defaultDock = dockable->getDefaultDock();
				panda::DockObject* newDock = defaultDock;
				int newIndex = -1;
				for(auto object : pandaDocument->getObjects())
				{
					panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object);
					if(dock)
					{
						if(dockableArea.intersects(objectDrawStructs[dock]->getObjectArea()) && dock->accepts(dockable))
						{
							newIndex = dynamic_cast<DockObjectDrawStruct*>(objectDrawStructs[dock].data())->getDockableIndex(dockableArea);
							newDock = dock;
							break;
						}
					}
				}

				panda::DockObject* prevDock = dockable->getParentDock();
				if(newDock != prevDock)
				{
					if(prevDock)
						prevDock->removeDockable(dockable);
					if(newDock)
						newDock->addDockable(dockable, newIndex);
					dockable->setParentDock(newDock);
				}
				else if(prevDock != defaultDock)
				{
					int prevIndex = prevDock->getIndexOfDockable(dockable);
					if(prevIndex != newIndex)
					{
						if(newIndex > prevIndex)
							--newIndex;
						prevDock->removeDockable(dockable);
						prevDock->addDockable(dockable, newIndex);
					}
					modifiedObject(prevDock);	// Always update
				}
			}
		}
	}
	else if(movingAction == MOVING_SELECTION)
	{
		pandaDocument->selectNone();

		QRectF selectionRect = QRectF(previousMousePos/zoomFactor, currentMousePos/zoomFactor).normalized();
		for(auto object : pandaDocument->getObjects())
		{
			QRectF objectArea = objectDrawStructs[object]->getObjectArea();
			if(selectionRect.contains(objectArea) || selectionRect.intersects(objectArea))
				pandaDocument->selectionAdd(object);
		}

		update();
	}
	else if(movingAction == MOVING_LINK)
	{
		panda::PandaObject* obj = getObjectAtPos(currentMousePos);
		if(obj)
		{
			panda::BaseData* secondData = objectDrawStructs[obj]->getDataAtPos(currentMousePos);
			if(secondData && isCompatible(clickedData, secondData))
			{
				if(clickedData->isInput() && secondData->isOutput())
					obj->dataSetParent(clickedData, secondData);
				else if(secondData->isInput() && clickedData->isOutput())
					obj->dataSetParent(secondData, clickedData);
				updateLinkTags();
			}
		}
		clickedData = nullptr;
		update();
	}
	else if(movingAction == MOVING_CUSTOM)
	{
		if(capturedDrawStruct)
		{
			capturedDrawStruct->mouseReleaseEvent(event);
			capturedDrawStruct = nullptr;
		}
	}

	QApplication::restoreOverrideCursor();
	movingAction = MOVING_NONE;
}

void GraphView::wheelEvent(QWheelEvent * event)
{
	if(movingAction != MOVING_NONE)
	{
		event->ignore();
		return;
	}

	int ticks = event->delta() / 40;
	int newZoom = qBound(0, zoomLevel-ticks, 90);
	if(zoomLevel != newZoom)
	{
		QPointF mousePos = event->pos();
		QPointF oldPos = mousePos / zoomFactor;
		zoomLevel = newZoom;
		zoomFactor = (100 - zoomLevel) / 100.0;
		moveView(mousePos / zoomFactor - oldPos);
		update();
	}
}

void GraphView::keyPressEvent(QKeyEvent * event)
{
	switch(event->key())
	{
	case Qt::Key_Space:
	{
		if(!pandaDocument->animationIsPlaying())
		{
			QuickCreateDialog dlg(pandaDocument, this);
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

void GraphView::contextMenuEvent(QContextMenuEvent *event)
{
	contextMenuData = nullptr;
	int flags = 0;
	QPointF zoomedMouse = event->pos() / zoomFactor;
	panda::PandaObject* object = getObjectAtPos(zoomedMouse);
	if(object)
	{
		flags |= MENU_OBJECT;
		panda::BaseData* data = objectDrawStructs[object]->getDataAtPos(zoomedMouse);
		if(data)
		{
			flags |= MENU_DATA;
			contextMenuData = data;
			if(contextMenuData->isInput() && contextMenuData->getParent())
				flags |= MENU_LINK;
		}
	}

	if(hoverTimer->isActive())
		hoverTimer->stop();

	emit showContextMenu(event->globalPos(), flags);
}

void GraphView::zoomIn()
{
	if(zoomLevel > 0)
	{
		QPointF center = contentsRect().center();
		QPointF oldPos = center / zoomFactor;
		zoomLevel = qMax(zoomLevel-10, 0);
		zoomFactor = (100 - zoomLevel) / 100.0;
		moveView(center / zoomFactor - oldPos);
		update();
	}
}

void GraphView::zoomOut()
{
	if(zoomLevel < 90)
	{
		QPointF center = contentsRect().center();
		QPointF oldPos = center / zoomFactor;
		zoomLevel = qMin(zoomLevel+10, 90);
		zoomFactor = (100 - zoomLevel) / 100.0;
		moveView(center / zoomFactor - oldPos);
		update();
	}
}

void GraphView::zoomReset()
{
	if(zoomLevel != 1)
	{
		QPointF center = contentsRect().center();
		QPointF oldPos = center / zoomFactor;
		zoomLevel = 1;
		zoomFactor = 1.0;
		moveView(center / zoomFactor - oldPos);
		update();
	}
}

void GraphView::centerView()
{
	if(pandaDocument->getNbObjects())
	{
		QRectF totalView;
		for(auto object : pandaDocument->getObjects())
		{
			QRectF objectArea = objectDrawStructs[object]->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		moveView(contentsRect().center() / zoomFactor - totalView.center());
		update();
	}
}

void GraphView::showAll()
{
	if(pandaDocument->getNbObjects())
	{
		QRectF totalView;
		for(auto object : pandaDocument->getObjects())
		{
			QRectF objectArea = objectDrawStructs[object]->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		qreal factorW = contentsRect().width() / (totalView.width() + 40);
		qreal factorH = contentsRect().height() / (totalView.height() + 40);
		zoomFactor = qBound(0.1, qMin(factorW, factorH), 1.0);
		zoomLevel = 100 * (1.0 - zoomFactor);
		moveView(contentsRect().center() / zoomFactor - totalView.center());
		update();
	}
}

void GraphView::showAllSelected()
{
	if(pandaDocument->getNbSelected())
	{
		QRectF totalView;
		for(auto object : pandaDocument->getSelection())
		{
			QRectF objectArea = objectDrawStructs[object]->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		qreal factorW = contentsRect().width() / (totalView.width() + 40);
		qreal factorH = contentsRect().height() / (totalView.height() + 40);
		zoomFactor = qBound(0.1, qMin(factorW, factorH), 1.0);
		zoomLevel = 100 * (1.0 - zoomFactor);
		moveView(contentsRect().center() / zoomFactor - totalView.center());
		update();
	}
}

void GraphView::moveSelectedToCenter()
{
	if(pandaDocument->getNbSelected())
	{
		QRectF totalView;
		for(auto object : pandaDocument->getSelection())
		{
			QRectF objectArea = objectDrawStructs[object]->getObjectArea();
			totalView = totalView.united(objectArea);
		}

		QPointF delta = contentsRect().center() / zoomFactor - totalView.center();

		for(auto object : pandaDocument->getSelection())
		{
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			// Do not move (docked) dockable objects, their parent dock move them already
			if(!dockable || !pandaDocument->isSelected(dockable->getParentDock()))
				objectDrawStructs[object]->move(delta);
		}

		update();
	}
}

void GraphView::addedObject(panda::PandaObject* object)
{
	ObjectDrawStruct* ods = ObjectDrawStructFactory::getInstance()->createDrawStruct(this, object);
	objectDrawStructs.insert(object, QSharedPointer<ObjectDrawStruct>(ods));

	update();
}

void GraphView::removeObject(panda::PandaObject* object)
{
	objectDrawStructs.remove(object);
	capturedDrawStruct = nullptr;
	movingAction = MOVING_NONE;
	linkTags.clear();
	recomputeTags = true;
	highlightConnectedDatas = false;
	update();
}

void GraphView::modifiedObject(panda::PandaObject* object)
{
	if(objectDrawStructs.contains(object))	// Can be called before the object is fully created
	{
		objectDrawStructs[object]->update();
		linkTags.clear();
		recomputeTags = true;
		highlightConnectedDatas = false;
		hoverData = nullptr;
		hoverTimer->stop();
		update();
	}
}

void GraphView::savingObject(QDomDocument& doc, QDomElement& elem, panda::PandaObject* object)
{
	objectDrawStructs[object]->save(doc, elem);
}

void GraphView::loadingObject(QDomElement& elem, panda::PandaObject* object)
{
	objectDrawStructs[object]->load(elem);
}

int GraphView::getAvailableLinkTagIndex()
{
	int nb = linkTags.size();
	QVector<bool> indices(nb, true);

	for(auto& tag : linkTags)
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
	return zoomFactor;
}

void GraphView::addLinkTag(panda::BaseData* input, panda::BaseData* output)
{
	if(linkTags.contains(input))
		linkTags[input]->addOutput(output);
	else
	{
		QSharedPointer<LinkTag> tag(new LinkTag(this, input, output));
		tag->index = getAvailableLinkTagIndex();
		linkTags[input] = tag;
	}
}

void GraphView::removeLinkTag(panda::BaseData* input, panda::BaseData* output)
{
	if(linkTags.contains(input))
	{
		LinkTag* tag = linkTags[input].data();
		tag->removeOutput(output);
		if(tag->isEmpty())
			linkTags.remove(input);
	}
}

void GraphView::updateLinkTags(bool reset)
{
	if(reset)
		linkTags.clear();

	for(auto object : pandaDocument->getObjects())
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

	QMutableMapIterator<panda::BaseData*, QSharedPointer<LinkTag> > iterTag(linkTags);
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
	if(contextMenuData && contextMenuData->isInput() && contextMenuData->getParent())
	{
		removeLinkTag(contextMenuData->getParent(), contextMenuData);
		contextMenuData->getOwner()->dataSetParent(contextMenuData, nullptr);
		contextMenuData = nullptr;
		update();
	}
}

void GraphView::hoverDataInfo()
{
	if(hoverData)
	{
		highlightConnectedDatas = true;
		update();
	}
}

void GraphView::drawConnectedDatas(QStylePainter* painter, panda::BaseData* sourceData)
{
	QVector<QRectF> highlightRects;
	QVector< QPair<QPointF, QPointF> > highlightLinks;

	QRectF sourceRect;
	if(objectDrawStructs[sourceData->getOwner()]->getDataRect(sourceData, sourceRect))
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
				if(objectDrawStructs.contains(object))
				{
					QRectF rect;
					if(objectDrawStructs[object]->getDataRect(data, rect))
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
			if(objectDrawStructs.contains(object))
			{
				QRectF rect;
				if(objectDrawStructs[object]->getDataRect(data, rect))
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
	snapTargetsX.clear();
	snapTargetsY.clear();

	auto viewRect = QRectF(contentsRect());

	// Use x position of every visible object
	for(auto ods : objectDrawStructs)
	{
		if(ods == selectedDrawStruct || !ods->acceptsMagneticSnap())
			continue;

		if(viewRect.intersects(ods->getObjectArea()))
		{
			auto pos = ods->getPosition();
			snapTargetsX.insert(pos.x());
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
					if(objectDrawStructs.contains(owner))
					{
						auto ods = objectDrawStructs[owner];
						if(ods->getDataRect(data2, dataRect))
							snapTargetsY.insert(dataRect.center().y() - dataHeight);
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
					if(objectDrawStructs.contains(owner))
					{
						auto ods = objectDrawStructs[owner];
						if(ods->getDataRect(data2, dataRect))
							snapTargetsY.insert(dataRect.center().y() - dataHeight);
					}
				}
			}
		}
	}
}

void GraphView::computeSnapDelta(QPointF position)
{
	snapDelta = QPointF();
	const qreal snapMaxDist = 5;

	// Give me c++14 lambdas !
	auto comparator = [](qreal pos) -> std::function<bool(const qreal& lhs, const qreal& rhs)>	{
		return [pos](const qreal& lhs, const qreal& rhs) {
			return qAbs(pos - lhs) < qAbs(pos - rhs);
		};
	};

	auto minIter = std::min_element(snapTargetsX.begin(), snapTargetsX.end(), comparator(position.x()));
	if(minIter != snapTargetsX.end())
	{
		qreal x = *minIter;
		if(qAbs(x - position.x()) < snapMaxDist)
			snapDelta.setX(x - position.x());
	}

	minIter = std::min_element(snapTargetsY.begin(), snapTargetsY.end(), comparator(position.y()));
	if(minIter != snapTargetsY.end())
	{
		qreal y = *minIter;
		if(qAbs(y - position.y()) < snapMaxDist)
			snapDelta.setY(y - position.y());
	}
}

//******************************************************************************************

LinkTag::LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output)
	: index(0)
	, hovering(false)
	, parentView(view)
	, inputData(input)
{
	outputDatas.insert(output, QRectF());
}

void LinkTag::addOutput(panda::BaseData* output)
{
	if(!outputDatas.contains(output))
		outputDatas.insert(output, QRectF());
}

void LinkTag::removeOutput(panda::BaseData* output)
{
	outputDatas.remove(output);
}

void LinkTag::update()
{
	QRectF dataRect = parentView->getDataRect(inputData);
	inputDataRect = QRectF(dataRect.right() + tagMargin,
						   dataRect.center().y() - tagH / 2.0,
						   tagW, tagH);

	qreal ix = dataRect.center().x();

	QMutableMapIterator<panda::BaseData*, QRectF> iter(outputDatas);
	while(iter.hasNext())
	{
		iter.next();
		dataRect = parentView->getDataRect(iter.key());
		qreal ox = dataRect.center().x();
		if(ix < ox)
			iter.remove();
		else
		{
			QRectF tagRect(dataRect.left() - tagW - tagMargin,
						   dataRect.center().y() - tagH / 2.0,
						   tagW, tagH);
			iter.setValue(tagRect);
		}
	}
}

bool LinkTag::isEmpty()
{
	return outputDatas.isEmpty();
}

bool LinkTag::isHovering(const QPointF& point)
{
	if(inputDataRect.contains(point))
		return true;

	for(auto rect : outputDatas.values())
		if(rect.contains(point))
			return true;

	return false;
}

void LinkTag::moveView(const QPointF& delta)
{
	inputDataRect.translate(delta);
	for(auto& rect : outputDatas)
		rect.translate(delta);
}

void LinkTag::draw(QPainter* painter)
{
	painter->save();
	if(hovering)
	{
		painter->setBrush(parentView->palette().highlight().color());
		QPen pen(parentView->palette().text().color());
		pen.setStyle(Qt::DotLine);
		painter->setPen(pen);

		for(auto tagRect : outputDatas.values())
			painter->drawLine(inputDataRect.center(), tagRect.center());
	}
	else
		painter->setBrush(parentView->palette().light());
	painter->setPen(QPen(parentView->palette().text().color()));

	QFont font;
	font.setPointSize(7);
	painter->setFont(font);

	// input
	qreal x = inputDataRect.left();
	qreal cy = inputDataRect.center().y();
	painter->drawLine(x - tagMargin, cy, x, cy);
	painter->drawRect(inputDataRect);
	painter->drawText(inputDataRect, Qt::AlignCenter, QString::number(index+1));

	// outputs
	for(auto tagRect : outputDatas.values())
	{
		x = tagRect.right();
		cy = tagRect.center().y();
		painter->drawLine(x, cy, x + tagMargin, cy);
		painter->drawRect(tagRect);
		painter->drawText(tagRect, Qt::AlignCenter, QString::number(index+1));
	}

	painter->restore();
}

panda::BaseData* LinkTag::getInputData()
{
	return inputData;
}
