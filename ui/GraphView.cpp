#include <QtWidgets>
#include <cmath>

#include <ui/GraphView.h>
#include <ui/ObjectDrawStruct.h>
#include <ui/QuickCreateDialog.h>
#include <ui/DockableDrawStruct.h>

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
	, recomputeTags(false)
{
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);

    connect(pandaDocument, SIGNAL(modified()), this, SLOT(update()));
    connect(pandaDocument, SIGNAL(selectionChanged()), this, SLOT(update()));
    connect(pandaDocument, SIGNAL(addedObject(panda::PandaObject*)), this, SLOT(addedObject(panda::PandaObject*)));
    connect(pandaDocument, SIGNAL(removedObject(panda::PandaObject*)), this, SLOT(removeObject(panda::PandaObject*)));
    connect(pandaDocument, SIGNAL(modifiedObject(panda::PandaObject*)), this, SLOT(modifiedObject(panda::PandaObject*)));
    connect(pandaDocument, SIGNAL(savingObject(QDataStream&, panda::PandaObject*)), this, SLOT(savingObject(QDataStream&, panda::PandaObject*)));
    connect(pandaDocument, SIGNAL(savingObject(QTextStream&, panda::PandaObject*)), this, SLOT(savingObject(QTextStream&, panda::PandaObject*)));
	connect(pandaDocument, SIGNAL(savingObject(QDomDocument&,QDomElement&,panda::PandaObject*)), this, SLOT(savingObject(QDomDocument&,QDomElement&,panda::PandaObject*)));
    connect(pandaDocument, SIGNAL(loadingObject(QDataStream&, panda::PandaObject*)), this, SLOT(loadingObject(QDataStream&, panda::PandaObject*)));
    connect(pandaDocument, SIGNAL(loadingObject(QTextStream&, panda::PandaObject*)), this, SLOT(loadingObject(QTextStream&, panda::PandaObject*)));
	connect(pandaDocument, SIGNAL(loadingObject(QDomElement&,panda::PandaObject*)), this, SLOT(loadingObject(QDomElement&,panda::PandaObject*)));

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

panda::PandaObject* GraphView::getObjectAtPos(const QPointF& pt)
{
    panda::PandaDocument::ObjectsIterator iter = pandaDocument->getObjectsIterator();
    iter.toBack();
    while(iter.hasPrevious())
    {
        panda::PandaObject* object = iter.previous();
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
	recomputeTags = false;
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
        foreach(QSharedPointer<ObjectDrawStruct> drawStruct, objectDrawStructs)
            drawStruct->moveVisual(delta);

        foreach(QSharedPointer<LinkTag> tag, linkTags.values())
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

    // Draw the objects
    panda::PandaDocument::ObjectsIterator iter = pandaDocument->getObjectsIterator();
    while(iter.hasNext())
        objectDrawStructs[iter.next()]->draw(&painter);

    // TEST : redraw selected objets in case they are moved over others (so that they don't appear under them)
    iter = pandaDocument->getSelectionIterator();
    while(iter.hasNext())
        objectDrawStructs[iter.next()]->draw(&painter);
    iter = pandaDocument->getObjectsIterator();

    // Draw links
    painter.setBrush(Qt::NoBrush);
    iter.toFront();
    while(iter.hasNext())
        objectDrawStructs[iter.next()]->drawLinks(&painter);

    // Draw links tags
    foreach(QSharedPointer<LinkTag> tag, linkTags.values())
        tag->draw(&painter);

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
}

void GraphView::resizeEvent(QResizeEvent * /* event */)
{
    update();
}

void GraphView::mousePressEvent(QMouseEvent *event)
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
                // Remove
                if(data->getParent() && event->modifiers() == Qt::ControlModifier)
                {
                    removeLinkTag(data->getParent(), data);
                    object->dataSetParent(data, nullptr);
                    emit modified();
                    update();
                    return;
                }
                else	// Creating a new Link
                {
                    clickedData = data;
                    movingAction = MOVING_LINK;
                    previousMousePos = currentMousePos = linkStart;

                    pandaDocument->selectNone();
                    return;
                }
            }
            else	// No Data, we still cliked on an object
            {
                // Add the object to the selection
                if(event->modifiers() == Qt::ControlModifier)
                {
                    if(pandaDocument->isSelected(object))
                        pandaDocument->selectionRemove(object);
                    else
                        pandaDocument->selectionAdd(object);
                    return;
                }
                else	// Moving the object (or selecting only this one if we release the mouse without moving)
                {
                    if(!pandaDocument->isSelected(object))
                        pandaDocument->selectNone();

                    pandaDocument->setCurrentSelectedObject(object);
                    movingAction = MOVING_START;
                    previousMousePos = zoomedMouse;
                    return;
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

void GraphView::mouseMoveEvent(QMouseEvent * event)
{
    if(movingAction == MOVING_START)
    {
		QPointF mousePos = event->localPos() / zoomFactor;
        QPointF delta = mousePos - previousMousePos;
        if(QVector2D(delta *  zoomFactor).length() > 5)
        {
            movingAction = MOVING_OBJECT;
            QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
            if(!delta.isNull())
            {
                panda::PandaDocument::ObjectsIterator iter = pandaDocument->getSelectionIterator();
                while(iter.hasNext())
                {
                    panda::PandaObject* object = iter.next();
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
        if(!delta.isNull())
        {
            panda::PandaDocument::ObjectsIterator iter = pandaDocument->getSelectionIterator();
            while(iter.hasNext())
            {
                panda::PandaObject* object = iter.next();
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
        update();
    }

    if(movingAction == MOVING_NONE || movingAction == MOVING_LINK)
    {
		QPointF zoomedMouse = event->localPos() / zoomFactor;
        panda::PandaObject* object = getObjectAtPos(zoomedMouse);
        if(object)
        {
            hoverData = objectDrawStructs[object]->getDataAtPos(zoomedMouse);
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

            if(movingAction == MOVING_NONE)
            {
                // Look for link tags
                QMapIterator<panda::BaseData*, QSharedPointer<LinkTag> > iter(linkTags);
                while(iter.hasNext())
                {
                    iter.next();
                    LinkTag* tag = iter.value().data();
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

void GraphView::mouseReleaseEvent(QMouseEvent * /*event*/)
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
        panda::PandaObject* object = pandaDocument->getCurrentSelectedObject();
        if(object)
        {
			panda::DockableObject* dockable = dynamic_cast<panda::DockableObject*>(object);
			if(dockable)
            {
				QRectF dockableArea = objectDrawStructs[dockable]->getObjectArea();
				panda::PandaDocument::ObjectsIterator iter = pandaDocument->getObjectsIterator();
				panda::DockObject* defaultDock = dockable->getDefaultDock();
				panda::DockObject* newDock = defaultDock;
                int newIndex = -1;
                while(iter.hasNext())
                {
					panda::DockObject* dock = dynamic_cast<panda::DockObject*>(iter.next());
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
        panda::PandaDocument::ObjectsIterator iter = pandaDocument->getObjectsIterator();
        while(iter.hasNext())
        {
            panda::PandaObject* object = iter.next();
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
                if(clickedData->isInput())
                    obj->dataSetParent(clickedData, secondData);
                else if(secondData->isInput())
                    obj->dataSetParent(secondData, clickedData);
                emit modified();
                updateLinkTags();
            }
        }
        clickedData = nullptr;
        update();
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
	if(event->key() == Qt::Key_Space)
	{
		QuickCreateDialog dlg(pandaDocument, this);
		dlg.exec();
	}
	else
		QWidget::keyPressEvent(event);
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
        panda::PandaDocument::ObjectsIterator iter = pandaDocument->getObjectsIterator();
        while(iter.hasNext())
        {
            panda::PandaObject* object = iter.next();
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
        panda::PandaDocument::ObjectsIterator iter = pandaDocument->getObjectsIterator();
        while(iter.hasNext())
        {
            panda::PandaObject* object = iter.next();
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
    panda::PandaDocument::ObjectsIterator iter = pandaDocument->getSelectionIterator();
    if(iter.hasNext())
    {
        QRectF totalView;
        while(iter.hasNext())
        {
            panda::PandaObject* object = iter.next();
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
    panda::PandaDocument::ObjectsIterator iter = pandaDocument->getSelectionIterator();
    if(iter.hasNext())
    {
        QRectF totalView;
        while(iter.hasNext())
        {
            panda::PandaObject* object = iter.next();
            QRectF objectArea = objectDrawStructs[object]->getObjectArea();
            totalView = totalView.united(objectArea);
        }

        QPointF delta = contentsRect().center() / zoomFactor - totalView.center();

        iter.toFront();
        while(iter.hasNext())
        {
            panda::PandaObject* object = iter.next();
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
    linkTags.clear();
	recomputeTags = true;
    update();
}

void GraphView::modifiedObject(panda::PandaObject* object)
{
    if(objectDrawStructs.contains(object))	// Can be called before the object is fully created
    {
        objectDrawStructs[object]->update();
		linkTags.clear();
		recomputeTags = true;
        update();
    }
}

void GraphView::savingObject(QDataStream& out, panda::PandaObject* object)
{
    out << objectDrawStructs[object]->getPosition();
}

void GraphView::savingObject(QTextStream& out, panda::PandaObject* object)
{
    QPointF pos = objectDrawStructs[object]->getPosition();
    out << pos.x() << " " << pos.y() << endl;
}

void GraphView::savingObject(QDomDocument&, QDomElement& elem, panda::PandaObject* object)
{
	QPointF pos = objectDrawStructs[object]->getPosition();
	elem.setAttribute("x", pos.x());
	elem.setAttribute("y", pos.y());
}

void GraphView::loadingObject(QDataStream& in, panda::PandaObject* object)
{
    QPointF newPos, prevPos;
    prevPos = objectDrawStructs[object]->getPosition();
    in >> newPos;
    objectDrawStructs[object]->move(newPos - prevPos);
}

void GraphView::loadingObject(QTextStream& in, panda::PandaObject* object)
{
    QPointF newPos, prevPos;
    prevPos = objectDrawStructs[object]->getPosition();
    in >> newPos.rx() >> newPos.ry();
    objectDrawStructs[object]->move(newPos - prevPos);
}

void GraphView::loadingObject(QDomElement& elem, panda::PandaObject* object)
{
	QPointF newPos, prevPos;
	prevPos = objectDrawStructs[object]->getPosition();
	newPos.setX(elem.attribute("x").toDouble());
	newPos.setY(elem.attribute("y").toDouble());
	objectDrawStructs[object]->move(newPos - prevPos);
}

int GraphView::getAvailableLinkTagIndex()
{
    int nb = linkTags.size();
    QVector<bool> indices(nb, true);

    QMapIterator<panda::BaseData*, QSharedPointer<LinkTag> > iterTag(linkTags);
    while(iterTag.hasNext())
    {
        iterTag.next();
        if(iterTag.value()->index < nb)
            indices[iterTag.value()->index] = false;
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

	panda::PandaDocument::ObjectsIterator iter = pandaDocument->getObjectsIterator();
    while(iter.hasNext())
    {
        panda::PandaObject* object = iter.next();
        foreach(panda::BaseData* data, object->getInputDatas())
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

    foreach(QRectF rect, outputDatas.values())
        if(rect.contains(point))
            return true;

    return false;
}

void LinkTag::moveView(const QPointF& delta)
{
    inputDataRect.translate(delta);
    for(QMap<panda::BaseData*, QRectF>::iterator iter=outputDatas.begin(); iter!=outputDatas.end(); ++iter)
        iter.value().translate(delta);
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

        foreach(QRectF tagRect, outputDatas.values())
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
    foreach(QRectF tagRect, outputDatas.values())
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
