#include <QtWidgets>
#include <QOpenGLFramebufferObject>

#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/DataFactory.h>
#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/Group.h>
#include <panda/helper/GradientCache.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

namespace panda {

PandaDocument::PandaDocument(QObject *parent)
	: PandaObject(parent)
	, currentIndex(1)
	, imageIsDirty(true)
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

PandaDocument::~PandaDocument()
{
	for(PandaObject* object : pandaObjects)
	{
		object->preDestruction();
		object->disconnect();
	}
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

	QDomDocument doc;
	QDomElement root = doc.createElement("Panda");
	doc.appendChild(root);
	save(doc, root);	// The document's Datas
	saveDoc(doc, root, pandaObjects);	// The document and all of its objects

	file.write(doc.toByteArray(4));

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

	QDomDocument doc;
	int errLine, errCol;
	if (!doc.setContent(&file, nullptr, &errLine, &errCol))
	{
		QMessageBox::warning(nullptr, tr("Panda"),
							 tr("Cannot parse xml file %1. Error in ligne %2, column %3")
							 .arg(file.fileName())
							 .arg(errLine)
							 .arg(errCol));
		return false;
	}

	QDomElement root = doc.documentElement();
	load(root);		// Only the document's Datas
	loadDoc(root);	// All the document's objects

	emit selectionChanged();
	emit selectedObject(getCurrentSelectedObject());

	return true;
}

QString PandaDocument::writeTextDocument()
{
	QDomDocument doc;
	QDomElement root = doc.createElement("Panda");
	doc.appendChild(root);

	saveDoc(doc, root, selectedObjects);

	return doc.toString(4);
}

bool PandaDocument::readTextDocument(QString& text)
{
	QDomDocument doc("Panda");
	if(!doc.setContent(text))
		return false;

	bool bSelected = !selectedObjects.isEmpty();
	selectedObjects.clear();

	QDomElement root = doc.documentElement();
	bool bVal = loadDoc(root);

	if(bSelected || !selectedObjects.empty())
	{
		emit selectionChanged();
		emit selectedObject(getCurrentSelectedObject());
	}

	return bVal;
}

bool PandaDocument::saveDoc(QDomDocument& doc, QDomElement& root, const QList<PandaObject*>& selected)
{
	typedef QPair<BaseData*, BaseData*> DataPair;
	QList<DataPair> links;

	typedef QPair<quint32, quint32> IntPair;
	QList<IntPair> dockedObjects;

	// Saving objects
	for(PandaObject* object : selected)
	{
		QDomElement elem = doc.createElement("Object");
		elem.setAttribute("type", ObjectFactory::getRegistryName(object));
		elem.setAttribute("index", object->getIndex());
		root.appendChild(elem);

		object->save(doc, elem, &selected);

		// Preparing links
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* parent = data->getParent();
			if(parent && selected.contains(parent->getOwner()))
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

		emit savingObject(doc, elem, object);
	}

	// Saving links
	for(DataPair link : links)
	{
		QDomElement elem = doc.createElement("Link");
		elem.setAttribute("object1", link.first->getOwner()->getIndex());
		elem.setAttribute("data1", link.first->getName());
		elem.setAttribute("object2", link.second->getOwner()->getIndex());
		elem.setAttribute("data2", link.second->getName());
		root.appendChild(elem);
	}

	// Saving docked objects list
	for(IntPair dockable : dockedObjects)
	{
		QDomElement elem = doc.createElement("Dock");
		elem.setAttribute("dock", dockable.first);
		elem.setAttribute("docked", dockable.second);
		root.appendChild(elem);
	}

	return true;
}

bool PandaDocument::loadDoc(QDomElement& root)
{
	QMap<quint32, quint32> importIndicesMap;

	// Loading objects
	QDomElement elem = root.firstChildElement("Object");
	while(!elem.isNull())
	{
		QString registryName = elem.attribute("type");
		if(registryName.isEmpty())
			return false;
		quint32 index = elem.attribute("index").toUInt();
		PandaObject* object = createObject(registryName);
		if(object)
		{
			importIndicesMap[index] = object->getIndex();
			selectedObjects.append(object);

			object->load(elem);

			emit loadingObject(elem, object);
		}
		else
		{
			QMessageBox::warning(nullptr, tr("Panda"),
				tr("Could not create the object %1.\nA plugin must be missing.")
				.arg(registryName));
			return false;
		}

		elem = elem.nextSiblingElement("Object");
	}

	// Create links
	elem = root.firstChildElement("Link");
	while(!elem.isNull())
	{
		quint32 index1, index2;
		QString name1, name2;
		index1 = elem.attribute("object1").toUInt();
		index2 = elem.attribute("object2").toUInt();
		index1 = importIndicesMap[index1];
		index2 = importIndicesMap[index2];

		name1 = elem.attribute("data1");
		name2 = elem.attribute("data2");

		BaseData *data1, *data2;
		data1 = findData(index1, name1);
		data2 = findData(index2, name2);
		if(data1 && data2)
			data1->setParent(data2);

		elem = elem.nextSiblingElement("Link");
	}

	// Put dockables in their docks
	elem = root.firstChildElement("Dock");
	while(!elem.isNull())
	{
		quint32 dockIndex, dockableIndex;
		dockIndex = elem.attribute("dock").toUInt();
		dockableIndex = elem.attribute("docked").toUInt();
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

		elem = elem.nextSiblingElement("Dock");
	}

	return true;
}

void PandaDocument::resetDocument()
{
	selectedObjects.clear();
	for(PandaObject* object : pandaObjects)
	{
		emit removedObject(object);
		object->preDestruction();
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

	animPlaying = false;
	animTimer->stop();

	emit modified();
	emit selectionChanged();
	emit selectedObject(nullptr);
	emit timeChanged();
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

const PandaDocument::ObjectsList PandaDocument::getObjects() const
{
	return pandaObjects;
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

const PandaDocument::ObjectsList PandaDocument::getSelection() const
{
	return selectedObjects;
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
	if(selectedObjects.isEmpty())
		return;
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
		ObjectsList selectedCopy = selectedObjects;
		selectedObjects.clear();

		for(PandaObject* object : selectedCopy)
			doRemoveObject(object);

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

			for(BaseData* data : object->getInputDatas())
			{
				if(data->getParent())
				{
					PandaObject* connected = data->getParent()->getOwner();
					if(!closedList.contains(connected))
						openList.insert(connected);
				}
			}

			for(BaseData* data : object->getOutputDatas())
			{
				for(DataNode* otherNode : data->getOutputs())
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

			DockableObject* dockable = dynamic_cast<DockableObject*>(object);
			if(dockable)
			{
				PandaObject* dock = dockable->getParentDock();
				if(dock != defaultLayer && !closedList.contains(dock))
					openList.insert(dock);
			}

			DockObject* dock = dynamic_cast<DockObject*>(object);
			if(dock)
			{
				auto iter = dock->getDockablesIterator();
				while(iter.hasNext())
				{
					PandaObject* docked = iter.next();
					if(!closedList.contains(docked))
						openList.insert(docked);
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
	{
		object->preDestruction();
		delete object;
	}
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

	QDomDocument doc;
	if(!doc.setContent(&file))
		return false;

	QDomElement root = doc.documentElement();
	if(!root.hasAttribute("description"))
		return false;
	description = root.attribute("description");

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
	if (!ok || text.isEmpty())
		return false;

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

	QDomDocument doc;
	QDomElement root = doc.createElement("Group");
	doc.appendChild(root);

	QString desc = QInputDialog::getText(nullptr, tr("Save group"),
										 tr("Group description:"), QLineEdit::Normal,
										 "", &ok);

	root.setAttribute("description", desc);
	root.setAttribute("type", panda::ObjectFactory::getRegistryName(group));

	group->save(doc, root);

	file.write(doc.toByteArray(4));
	return true;
}

PandaObject* PandaDocument::createGroupObject(QString groupPath)
{
	QFile file(groupsDirPath + "/" + groupPath + ".grp");
	if(!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, tr("Panda"), tr("Could not open the file."));
		return nullptr;
	}

	QDomDocument doc;
	int errLine, errCol;
	if (!doc.setContent(&file, nullptr, &errLine, &errCol))
	{
		QMessageBox::warning(nullptr, tr("Panda"),
							 tr("Cannot parse xml: error in ligne %2, column %3")
							 .arg(errLine)
							 .arg(errCol));
		return false;
	}

	QDomElement root = doc.documentElement();
	QString description = root.attribute("description");
	QString registryName = root.attribute("type");

	panda::PandaObject* object = createObject(registryName);
	if(object)
		object->load(root);
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
	if(object == getCurrentSelectedObject())
		emit selectedObjectIsDirty(object);
	emit modified();
}

void PandaDocument::update()
{
	if(!renderFrameBuffer || renderFrameBuffer->size() != getRenderSize())
	{
		QOpenGLFramebufferObjectFormat fmt;
		fmt.setSamples(16);
		renderFrameBuffer.reset(new QOpenGLFramebufferObject(getRenderSize(), fmt));
		displayFrameBuffer.reset(new QOpenGLFramebufferObject(getRenderSize()));
	}

	defaultLayer->updateIfDirty();

	for(auto obj : pandaObjects)
	{
		if(dynamic_cast<BaseLayer*>(obj))
			obj->updateIfDirty();
	}

	renderFrameBuffer->bind();
	renderOpenGL();
	renderFrameBuffer->release();

	// We have to blit the document's multisample fbo to another fbo
	QOpenGLFramebufferObject::blitFramebuffer(displayFrameBuffer.data(), renderFrameBuffer.data());

	imageIsDirty = true;
	this->cleanDirty();
}

const QImage& PandaDocument::getRenderedImage()
{
	this->updateIfDirty();
	if(imageIsDirty)
	{
		renderedImage = displayFrameBuffer->toImage();
		imageIsDirty = false;
	}

	return renderedImage;
}

QOpenGLFramebufferObject* PandaDocument::getFBO()
{
	this->updateIfDirty();

	return displayFrameBuffer.data();
}

void PandaDocument::renderOpenGL()
{
	glViewport(0, 0, renderFrameBuffer->width(), renderFrameBuffer->height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, renderFrameBuffer->width(), renderFrameBuffer->height(), 0, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	QColor col = backgroundColor.getValue();
	glClearColor(col.redF(), col.greenF(), col.blueF(), col.alphaF());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	defaultLayer->mergeLayer();

	for(auto obj : pandaObjects)
	{
		BaseLayer* layer = dynamic_cast<BaseLayer*>(obj);
		if(layer)
			layer->mergeLayer();
	}
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
	PandaObject::setDirtyValue();
	if(!getCurrentSelectedObject())
		emit selectedObjectIsDirty(nullptr);
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
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->startLog(this);
#endif
	helper::GradientCache::getInstance()->setUsedFlag();

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
	updateIfDirty();

	helper::GradientCache::getInstance()->clearUnused();
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->stopLog();
#endif
	emit timeChanged();
}

void PandaDocument::rewind()
{
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->startLog(this);
#endif
	animTime.setValue(0.0);
	mousePosition.setValue(mousePositionBuffer);
	mouseClick.setValue(0);
	for(PandaObject* object : pandaObjects)
		object->reset();
	setDirtyValue();
	emit timeChanged();
}

void PandaDocument::copyDataToUserValue(const BaseData* data)
{
	if(!data)
		return;

	auto entry = DataFactory::getInstance()->getEntry(data->getDataTrait()->fullTypeId());
	if(!entry)
		return;

	QString registryName = QString("panda::GeneratorUser<") + entry->className + ">";
	PandaObject* object = createObject(registryName);

	BaseData* valueData = object->getData("value");
	if(valueData)
	{
		valueData->copyValueFrom(data);
		valueData->setWidget(data->getWidget());
		valueData->setWidgetData(data->getWidgetData());
	}
}

} // namespace panda


