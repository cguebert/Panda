#include <QtWidgets>

#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/Group.h>

namespace panda {

PandaDocument::PandaDocument(QObject *parent)
    : PandaObject(parent)
    , currentIndex(1)
    , renderSize(initData(&renderSize, QPointF(800,600), "render size", "Size of the image to be rendered"))
    , backgroundColor(initData(&backgroundColor, QColor(255,255,255), "background color", "Background color of the image to be rendered"))
    , animTime(initData(&animTime, 0.0, "time", "Time of the animation"))
    , timestep(initData(&timestep, 0.1, "timestep", "Time step of the animation"))
	, mousePosition(initData(&mousePosition, "mousePosition", "Current position of the mouse in the render view"))
	, mouseClick(initData(&mouseClick, 0, "mouseClick", "1 if the left mouse button is pressed"))
	, mouseClickBuffer(0)
    , animPlaying(false)
{
    addInput(&renderSize);
    addInput(&backgroundColor);
    addInput(&timestep);

    // Not connecting to the document, otherwise it would update the layers each time we get the time.
    animTime.setOutput(true);
    animTime.setReadOnly(true);

	mousePosition.setOutput(true);
    mousePosition.setReadOnly(true);

	mouseClick.setOutput(true);
    mouseClick.setReadOnly(true);
	mouseClick.setWidget("checkbox");

    connect(this, SIGNAL(modifiedObject(panda::PandaObject*)), this, SIGNAL(modified()));
    connect(this, SIGNAL(addedObject(panda::PandaObject*)), this, SIGNAL(modified()));

    defaultLayer = new Layer(this);
    defaultLayer->setInternalData("Default Layer", 0);

    setInternalData("Document", 0);

    animTimer = new QTimer(this);
    connect(animTimer, SIGNAL(timeout()), this, SLOT(step()));

    groupsDirPath = QCoreApplication::applicationDirPath() + "/groups/";
}


bool PandaDocument::writeFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(nullptr, tr("Panda"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_6);

    out << pandaMagicNumber;
    out << pandaVersion;

    this->save(out);

    typedef QPair<BaseData*, BaseData*> DataPair;
    QList<DataPair> links;

    typedef QPair<quint32, quint32> IntPair;
	QList<IntPair> dockedObjects;

    // Saving objects
    out << (quint32)pandaObjects.size();
    QList<PandaObject*>::iterator iter;
    for(iter = pandaObjects.begin(); iter != pandaObjects.end(); ++iter)
    {
        PandaObject* object = *iter;
		out << ObjectFactory::getRegistryName(object);
        out << object->getIndex();

        object->save(out);

        // Preparing links
        foreach(BaseData* data, object->getInputDatas())
        {
            BaseData* parent = data->getParent();
            if(parent)
                links.append(qMakePair(data, parent));
        }

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
        {
			DockObject::DockablesIterator dockableIter = dock->getDockablesIterator();
			while(dockableIter.hasNext())
				dockedObjects.append(qMakePair(dock->getIndex(), dockableIter.next()->getIndex()));
        }

        emit savingObject(out, object);
    }

    // Saving links
    out << (quint32)links.size();
    foreach(DataPair link, links)
    {
        out << link.first->getOwner()->getIndex();
        out << link.first->getName();
        out << link.second->getOwner()->getIndex();
        out << link.second->getName();
    }

	// Saving docked objects list
	out << (quint32)dockedObjects.size();
	foreach(IntPair dockable, dockedObjects)
		out << dockable.first << dockable.second;

    return true;
}

bool PandaDocument::readFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, tr("Panda"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_6);

    quint32 magicNumber, version;
    in >> magicNumber;
    in >> version;
    if(magicNumber != pandaMagicNumber)
    {
        QMessageBox::warning(nullptr, tr("Panda"), tr("The file is not a Panda file."));
        return false;
    }

    if(version < pandaVersion)
    {
        QMessageBox::warning(nullptr, tr("Panda"), tr("This version is not supported any more."));
        return false;
    }

    this->load(in);

    QMap<quint32, quint32> importIndicesMap;
    selectedObjects.clear();

    quint32 nbObjects;
    in >> nbObjects;
    for(quint32 i=0; i<nbObjects; ++i)
    {
        QString registryName;
        quint32 index;
        in >> registryName >> index;

        PandaObject* object = createObject(registryName);
        if(object)
        {
            importIndicesMap[index] = object->getIndex();
            selectedObjects.append(object);

            object->load(in);

            emit loadingObject(in, object);
        }
        else
        {
            QMessageBox::warning(nullptr, tr("Panda"),
                tr("Could not create the object %1.\nA plugin must be missing.")
                .arg(registryName));
            return false;
        }
    }

    // Create links
    quint32 nbLinks;
    in >> nbLinks;
    for(quint32 i=0; i<nbLinks; ++i)
    {
        quint32 index1, index2;
        QString name1, name2;
        in >> index1 >> name1 >> index2 >> name2;
        index1 = importIndicesMap[index1];
        index2 = importIndicesMap[index2];

        BaseData *data1, *data2;
        data1 = findData(index1, name1);
        data2 = findData(index2, name2);
        if(data1 && data2)
            data1->setParent(data2);
    }

	// Put dockables in their docks
	quint32 nbDockedObjects;
	in >> nbDockedObjects;
	for(quint32 i=0; i<nbDockedObjects; ++i)
    {
		quint32 dockIndex, dockableIndex;
		in >> dockIndex >> dockableIndex;
		dockIndex = importIndicesMap[dockIndex];
		dockableIndex = importIndicesMap[dockableIndex];

		DockObject* dock = dynamic_cast<DockObject*>(findObject(dockIndex));
		DockableObject* dockable = dynamic_cast<DockableObject*>(findObject(dockableIndex));
		if(dock && dockable)
        {
			DockObject* defaultDock = dockable->getDefaultDock();
			if(defaultDock)
				defaultDock->removeDockable(dockable);
			dock->addDockable(dockable);
        }
    }

    if(!selectedObjects.empty())
    {
        emit selectionChanged();
        emit selectedObject(getCurrentSelectedObject());
    }

    return true;
}

QString PandaDocument::writeTextDocument()
{
    QString text;
    QTextStream out(&text, QIODevice::WriteOnly);

    typedef QPair<BaseData*, BaseData*> DataPair;
    QList<DataPair> links;

    typedef QPair<quint32, quint32> IntPair;
	QList<IntPair> dockedObjects;

    // Saving objects
    out << (quint32)selectedObjects.size() << endl;
    QList<PandaObject*>::iterator iter;
    for(iter = selectedObjects.begin(); iter != selectedObjects.end(); ++iter)
    {
        PandaObject* object = *iter;
		out << ObjectFactory::getRegistryName(object) << endl;
        out << object->getIndex() << endl;

        object->save(out);

        // Preparing links
        foreach(BaseData* data, object->getInputDatas())
        {
            BaseData* parent = data->getParent();
			if(parent && selectedObjects.contains(parent->getOwner()))
                links.append(qMakePair(data, parent));
        }

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			DockObject::DockablesIterator dockableIter = dock->getDockablesIterator();
			while(dockableIter.hasNext())
				dockedObjects.append(qMakePair(dock->getIndex(), dockableIter.next()->getIndex()));
		}

        emit savingObject(out, object);
    }

    // Saving links
    out << (quint32)links.size() << endl;
    foreach(DataPair link, links)
    {
        out << link.first->getOwner()->getIndex() << " ";
        out << link.first->getName() << endl;
        out << link.second->getOwner()->getIndex() << " ";
        out << link.second->getName() << endl;
    }

	// Saving docked objects list
	out << (quint32)dockedObjects.size() << endl;
	foreach(IntPair dockable, dockedObjects)
		out << dockable.first << " " << dockable.second << endl;

    return text;
}

bool PandaDocument::readTextDocument(QString& text)
{
    QTextStream in(&text, QIODevice::ReadOnly);

    QMap<quint32, quint32> importIndicesMap;
    selectedObjects.clear();

    quint32 nbObjects;
    in >> nbObjects;
    for(quint32 i=0; i<nbObjects; ++i)
    {
        QString registryName;
        quint32 index;
        in.skipWhiteSpace();
        registryName = in.readLine();
        in >> index;

        PandaObject* object = createObject(registryName);
        if(object)
        {
            importIndicesMap[index] = object->getIndex();
            selectedObjects.append(object);

            object->load(in);

            emit loadingObject(in, object);
        }
        else
        {
            QMessageBox::warning(nullptr, tr("Panda"),
                tr("Could not create the object %1.\nA plugin must be missing.")
                .arg(registryName));
            return false;
        }
    }

    // Create links
    quint32 nbLinks;
    in >> nbLinks;
    for(quint32 i=0; i<nbLinks; ++i)
    {
        quint32 index1, index2;
        QString name1, name2;
        in >> index1;
        in.skipWhiteSpace();
        name1 = in.readLine();
        in >> index2;
        in.skipWhiteSpace();
        name2 = in.readLine();
        index1 = importIndicesMap[index1];
        index2 = importIndicesMap[index2];

        BaseData *data1, *data2;
        data1 = findData(index1, name1);
        data2 = findData(index2, name2);
        if(data1 && data2)
            data1->setParent(data2);
    }

	// Put dockables in their docks
	quint32 nbDockedObjects;
	in >> nbDockedObjects;
	for(quint32 i=0; i<nbDockedObjects; ++i)
	{
		quint32 dockIndex, dockableIndex;
		in >> dockIndex >> dockableIndex;
		dockIndex = importIndicesMap[dockIndex];
		dockableIndex = importIndicesMap[dockableIndex];

		DockObject* dock = dynamic_cast<DockObject*>(findObject(dockIndex));
		DockableObject* dockable = dynamic_cast<DockableObject*>(findObject(dockableIndex));
		if(dock && dockable)
		{
			DockObject* defaultDock = dockable->getDefaultDock();
			if(defaultDock)
				defaultDock->removeDockable(dockable);
			dock->addDockable(dockable);
		}
	}

    if(!selectedObjects.empty())
    {
        emit selectionChanged();
        emit selectedObject(getCurrentSelectedObject());
    }

    return true;
}

void PandaDocument::resetDocument()
{
    selectedObjects.clear();
    foreach(PandaObject* object, pandaObjects)
    {
		emit removedObject(object);
        object->disconnect(this);
        delete object;
    }

    pandaObjectsMap.clear();
    pandaObjects.clear();
    currentIndex = 1;
    animTime.setValue(0.0);
    timestep.setValue(0.1);
	renderSize.setValue(QPointF(800,600));
	backgroundColor.setValue(QColor(255,255,255));

    emit modified();
    emit selectionChanged();
    emit selectedObject(nullptr);
}

PandaObject* PandaDocument::createObject(QString registryName)
{
    PandaObject* object = ObjectFactory::getInstance()->create(registryName, this);
    if(object)
        doAddObject(object);

    return object;
}

int PandaDocument::getNbObjects() const
{
    return pandaObjects.size();
}

PandaDocument::ObjectsIterator PandaDocument::getObjectsIterator() const
{
    return ObjectsIterator(pandaObjects);
}

PandaObject* PandaDocument::getCurrentSelectedObject()
{
    if(selectedObjects.empty())
        return nullptr;
    else
        return selectedObjects.back();
}

void PandaDocument::setCurrentSelectedObject(PandaObject* object)
{
    selectedObjects.removeAll(object);
    selectedObjects.append(object);
    emit selectedObject(object);
    emit selectionChanged();
}

bool PandaDocument::isSelected(PandaObject* object) const
{
    return selectedObjects.contains(object);
}

int PandaDocument::getNbSelected() const
{
    return selectedObjects.size();
}

PandaDocument::ObjectsIterator PandaDocument::getSelectionIterator() const
{
    return ObjectsIterator(selectedObjects);
}

QColor PandaDocument::getBackgroundColor()
{
    return backgroundColor.getValue();
}

void PandaDocument::setBackgroundColor(QColor color)
{
    backgroundColor.setValue(color);
}

QSize PandaDocument::getRenderSize()
{
    QPointF pt = renderSize.getValue();
    return QSize(qMax(1, (int)pt.x()), qMax(1, (int)pt.y()));
}

double PandaDocument::getAnimationTime()
{
    return animTime.getValue();
}

double PandaDocument::getTimeStep()
{
	return timestep.getValue();
}

QPointF PandaDocument::getMousePosition()
{
	return mousePosition.getValue();
}

void PandaDocument::setMousePosition(const QPointF& pos)
{
	mousePositionBuffer = pos;
}

int PandaDocument::getMouseClick()
{
	return mouseClick.getValue();
}

void PandaDocument::setMouseClick(int state)
{
	if(mouseClickBuffer && !state) // Pressed & released in 1 timestep, we will send 2 events
		mouseClickBuffer = -1;
	else
		mouseClickBuffer = state;
}

void PandaDocument::cut()
{
    copy();
    del();
}

void PandaDocument::copy()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(writeTextDocument());
}

void PandaDocument::paste()
{
    const QClipboard* clipboard = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    if(mimeData->hasText())
    {
        QString clipText = mimeData->text();
        readTextDocument(clipText);
    }
}

void PandaDocument::del()
{
    if(!selectedObjects.empty())
    {
        foreach(PandaObject* object, selectedObjects)
            doRemoveObject(object);

        selectedObjects.clear();
        emit selectedObject(nullptr);
        emit selectionChanged();
        emit modified();
    }
}

void PandaDocument::selectionAdd(PandaObject* object)
{
    if(!selectedObjects.contains(object))
    {
        selectedObjects.append(object);
        emit selectedObject(object);
        emit selectionChanged();
    }
}

void PandaDocument::selectionRemove(PandaObject* object)
{
    if(selectedObjects.contains(object))
    {
        selectedObjects.removeAll(object);
        emit selectedObject(selectedObjects.back());
        emit selectionChanged();
    }
}

void PandaDocument::selectAll()
{
    selectedObjects = pandaObjects;
    emit selectedObject(selectedObjects.back());
    emit selectionChanged();
}

void PandaDocument::selectNone()
{
    if(!selectedObjects.empty())
    {
        selectedObjects.clear();
        emit selectedObject(nullptr);
        emit selectionChanged();
    }
}

void PandaDocument::selectConnected()
{
    if(!selectedObjects.empty())
    {
        QSet<PandaObject*> closedList, openList;
        openList = QSet<PandaObject*>::fromList(selectedObjects);
        while(!openList.empty())
        {
            PandaObject* object = *openList.begin();
            openList.remove(object);
            closedList.insert(object);

            foreach(BaseData* data, object->getInputDatas())
            {
                if(data->getParent())
                {
                    PandaObject* connected = data->getParent()->getOwner();
                    if(!closedList.contains(connected))
                        openList.insert(connected);
                }
            }

            foreach(BaseData* data, object->getOutputDatas())
            {
                foreach(DataNode* otherNode, data->getOutputs())
                {
                    BaseData* otherData = dynamic_cast<BaseData*>(otherNode);
                    if(otherData)
                    {
                        PandaObject* connected = otherData->getOwner();
                        if(!closedList.contains(connected))
                            openList.insert(connected);
                    }
                }
            }
        }

        PandaObject* currentSelected = selectedObjects.back();
        selectedObjects = closedList.toList();
        setCurrentSelectedObject(currentSelected);
        emit selectionChanged();
    }
}

void PandaDocument::doRemoveObject(PandaObject* object, bool del)
{
	emit removedObject(object);
    pandaObjectsMap.remove(object->getIndex());
    pandaObjects.removeAll(object);
    selectedObjects.removeAll(object);
    object->disconnect(this);
    if(del)
        delete object;
}

void PandaDocument::doAddObject(PandaObject* object)
{
    pandaObjectsMap.insert(object->getIndex(), object);
    pandaObjects.append(object);
    connect(object, SIGNAL(modified(panda::PandaObject*)), this, SIGNAL(modifiedObject(panda::PandaObject*)));
    connect(object, SIGNAL(dirty(panda::PandaObject*)), this, SLOT(onDirtyObject(panda::PandaObject*)));
    emit addedObject(object);
}

void PandaDocument::createGroupsList()
{
    groupsMap.clear();
    QStringList nameFilter;
    nameFilter << "*.grp";

    QStack<QString> dirList;
    dirList.push(groupsDirPath);
    QDir groupsDir(groupsDirPath);

    while(!dirList.isEmpty())
    {
        QDir dir = QDir(dirList.pop());
        QFileInfoList entries = dir.entryInfoList(nameFilter, QDir::Files);
        for(int i=0, nb=entries.size(); i<nb; i++)
        {
            QString desc;
            if(getGroupDescription(entries[i].absoluteFilePath(), desc))
            {
                QString path = groupsDir.relativeFilePath(entries[i].absoluteFilePath());
                int n = path.lastIndexOf(".grp", -1, Qt::CaseInsensitive);
                if(n != -1)
                    path = path.left(n);
                groupsMap[path] = desc;
            }
        }

        entries = dir.entryInfoList(QStringList(),
            QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        for(int i=0, nb=entries.size(); i<nb; i++)
            dirList.push(entries[i].absoluteFilePath());
    }
}

PandaDocument::GroupsIterator PandaDocument::getGroupsIterator()
{
    return GroupsIterator(groupsMap);
}

bool PandaDocument::getGroupDescription(const QString &fileName, QString& description)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_6);

    quint32 magicNumber, version;
    in >> magicNumber;
    in >> version;
    if(magicNumber != groupsMagicNumber)
        return false;

    if(version < groupsVersion)
        return false;

    in >> description;

    return true;
}

QString PandaDocument::getGroupDescription(const QString& groupName)
{
    return groupsMap.value(groupName);
}

bool PandaDocument::saveGroup(Group *group)
{
    bool ok;
    QString text = QInputDialog::getText(nullptr, tr("Save group"),
                                         tr("Group name:"), QLineEdit::Normal,
                                         group->getGroupName(), &ok);
    if (ok && !text.isEmpty())
    {
        QString fileName = groupsDirPath + text + ".grp";
        QFileInfo fileInfo(fileName);
        QDir dir;
        dir.mkpath(fileInfo.dir().path());
        QFile file(fileName);

        // If already exists
        if(file.exists())
        {
            if(QMessageBox::question(nullptr, tr("Panda"),
                                  tr("This group already exists, overwrite?"),
                                  QMessageBox::Yes|QMessageBox::No,
                                  QMessageBox::Yes)
                    != QMessageBox::Yes)
                return false;
        }

        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::warning(nullptr, tr("Panda"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(file.fileName())
                                 .arg(file.errorString()));
            return false;
        }

        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_4_6);

        out << groupsMagicNumber;
        out << groupsVersion;

        QString text = QInputDialog::getText(nullptr, tr("Save group"),
                                             tr("Group description:"), QLineEdit::Normal,
                                             "", &ok);
        out << text;

        out << panda::ObjectFactory::getRegistryName(group);
        group->save(out);
        return true;
    }

    return false;
}

PandaObject* PandaDocument::createGroupObject(QString groupPath)
{
    QFile file(groupsDirPath + "/" + groupPath + ".grp");
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(nullptr, tr("Panda"), tr("Could not open the file."));
        return nullptr;
    }

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_6);

    quint32 magicNumber, version;
    in >> magicNumber;
    in >> version;
    if(magicNumber != groupsMagicNumber)
    {
        QMessageBox::warning(nullptr, tr("Panda"), tr("The file is not a Panda file."));
        return nullptr;
    }

    if(version < groupsVersion)
    {
        QMessageBox::warning(nullptr, tr("Panda"), tr("This version is not supported any more."));
        return nullptr;
    }

    QString description;
    in >> description;

    QString registryName;
    in >> registryName;

    panda::PandaObject* object = createObject(registryName);
    if(object)
    {
        object->load(in);
    }
    else
    {
        QMessageBox::warning(nullptr, tr("Panda"),
            tr("Could not create the object %1.\nA plugin must be missing.")
            .arg(registryName));
        return nullptr;
    }

    return object;
}

quint32 PandaDocument::getNextIndex()
{
    return currentIndex++;
}

PandaObject* PandaDocument::findObject(quint32 objectIndex)
{
    if(pandaObjectsMap.contains(objectIndex))
        return pandaObjectsMap[objectIndex];

    return nullptr;
}

BaseData* PandaDocument::findData(quint32 objectIndex, const QString& dataName)
{
    PandaObject* object = findObject(objectIndex);
    if(object)
        return object->getData(dataName);

    return nullptr;
}

void PandaDocument::onDirtyObject(panda::PandaObject* object)
{
    PandaObject* selected = getCurrentSelectedObject();
    if(object == selected)
        emit selectedObjectIsDirty(object);
    emit modified();
}

void PandaDocument::update()
{
    if(getRenderSize() != renderedImage.size())
        renderedImage = QImage(getRenderSize(), QImage::Format_ARGB32);
    renderedImage.fill(backgroundColor.getValue());

    QPainter painter(&renderedImage);
	defaultLayer->updateIfDirty();
    defaultLayer->mergeLayer(&painter);

	ObjectsIterator iter = getObjectsIterator();
	while(iter.hasNext())
	{
		PandaObject* obj = iter.next();
		BaseLayer* layer = dynamic_cast<BaseLayer*>(obj);
		if(layer)
		{
			obj->updateIfDirty();
			layer->mergeLayer(&painter);
		}
	}

    this->cleanDirty();
}

const QImage& PandaDocument::getRenderedImage()
{
    this->updateIfDirty();
    return renderedImage;
}

Layer* PandaDocument::getDefaultLayer()
{
    return defaultLayer;
}

void PandaDocument::moveLayerUp(PandaObject* layer)
{
	if(!layer)
		return;
	int index = pandaObjects.indexOf(layer);
	if(index == -1)
		return;
	int nb = pandaObjects.size();
	for(++index;index<nb;++index)
	{
		BaseLayer* otherLayer = dynamic_cast<BaseLayer*>(pandaObjects.at(index));
		if(otherLayer)
		{
			pandaObjects.removeAll(layer);
			pandaObjects.insert(index, layer);
			setDirtyValue();
			emit modified();
			return;
		}
	}
}

void PandaDocument::moveLayerDown(PandaObject *layer)
{
	if(!layer)
		return;
	int index = pandaObjects.indexOf(layer);
	if(index == -1)
		return;
	for(--index;index>=0;--index)
	{
		BaseLayer* otherLayer = dynamic_cast<BaseLayer*>(pandaObjects.at(index));
		if(otherLayer)
		{
			pandaObjects.removeAll(layer);
			pandaObjects.insert(index, layer);
			setDirtyValue();
			emit modified();
			return;
		}
	}
}

void PandaDocument::setDirtyValue()
{
    if(!this->dirtyValue)
    {
        DataNode::setDirtyValue();
        if(!getCurrentSelectedObject())
            emit selectedObjectIsDirty(nullptr);
        emit modified();
    }
}

void PandaDocument::play(bool playing)
{
    animPlaying = playing;
    if(playing)
        animTimer->start(timestep.getValue() * 1000);
    else
        animTimer->stop();
}

void PandaDocument::step()
{
    animTime.setValue(animTime.getValue() + timestep.getValue());
	mousePosition.setValue(mousePositionBuffer);
	if(mouseClickBuffer < 0)
	{
		mouseClick.setValue(1);
		mouseClickBuffer = 0;
	}
	else
		mouseClick.setValue(mouseClickBuffer);
    setDirtyValue();
    emit timeChanged();
}

void PandaDocument::rewind()
{
    animTime.setValue(0.0);
	mousePosition.setValue(mousePositionBuffer);
	mouseClick.setValue(0);
    foreach(PandaObject* object, pandaObjects)
        object->reset();
    setDirtyValue();
    emit timeChanged();
}

} // namespace panda


