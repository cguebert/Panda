#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/DataFactory.h>
#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/Scheduler.h>
#include <panda/helper/GradientCache.h>
#include <panda/helper/ShaderCache.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

#include <ui/command/AddObjectCommand.h>

#include <QtWidgets>
#include <QOpenGLFramebufferObject>
#include <QElapsedTimer>
#include <QUndoStack>
#include <QDomDocument>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Point;

template<class C, class V>
bool contains(C& container, V value)
{ return std::find(std::begin(container), std::end(container), value) != std::end(container); }

template<class C, class V>
void remove(C& container, V value)
{
	auto iter = std::find(std::begin(container), std::end(container), value);
	if(iter != container.end())
		container.erase(iter);
}

PandaDocument::PandaDocument(QObject* parent)
	: PandaObject(nullptr)
	, QObject(parent)
	, currentIndex(1)
	, renderSize(initData(&renderSize, Point(800,600), "render size", "Size of the image to be rendered"))
	, backgroundColor(initData(&backgroundColor, Color::white(), "background color", "Background color of the image to be rendered"))
	, animTime(initData(&animTime, (PReal)0.0, "time", "Time of the animation"))
	, timestep(initData(&timestep, (PReal)0.01, "timestep", "Time step of the animation"))
	, useTimer(initData(&useTimer, 1, "use timer", "If true, wait before the next timestep. If false, compute the next one as soon as the previous finished."))
	, mousePosition(initData(&mousePosition, "mouse position", "Current position of the mouse in the render view"))
	, mouseClick(initData(&mouseClick, 0, "mouse click", "1 if the left mouse button is pressed"))
	, renderedImage(initData(&renderedImage, "rendered image", "Current image displayed"))
	, useMultithread(initData(&useMultithread, 0, "use multithread", "Optimize computation for multiple CPU cores"))
	, mouseClickBuffer(0)
	, animPlaying(false)
	, animMultithread(false)
	, m_inCommandMacro(0)
{
	addInput(&renderSize);
	addInput(&backgroundColor);
	addInput(&timestep);
	addInput(&useTimer);

	useTimer.setWidget("checkbox");

	// Not connecting to the document, otherwise it would update the layers each time we get the time.
	animTime.setOutput(true);
	animTime.setReadOnly(true);

	mousePosition.setOutput(true);
	mousePosition.setReadOnly(true);

	mouseClick.setOutput(true);
	mouseClick.setReadOnly(true);
	mouseClick.setWidget("checkbox");

	useMultithread.setWidget("checkbox");

	connect(this, SIGNAL(modifiedObject(panda::PandaObject*)), this, SIGNAL(modified()));
	connect(this, SIGNAL(addedObject(panda::PandaObject*)), this, SIGNAL(modified()));

	defaultLayer = new Layer(this);
	defaultLayer->setInternalData("Default Layer", 0);
	defaultLayer->setLayerName("Default Layer");

	setInternalData("Document", 0);

	animTimer = new QTimer(this);
	connect(animTimer, SIGNAL(timeout()), this, SLOT(step()));

	m_undoStack = new QUndoStack(this);
	m_undoStack->setUndoLimit(25);
}

PandaDocument::~PandaDocument()
{
	if(m_scheduler)
		m_scheduler->stop();
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
	ObjectsSelection allObjects;
	for(auto object : pandaObjects)
		allObjects.push_back(object.data());
	saveDoc(doc, root, allObjects);	// The document and all of its objects

	file.write(doc.toByteArray(4));

	return true;
}

bool PandaDocument::readFile(const QString& fileName, bool isImport)
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
	if(!isImport)	// Bugfix: don't read the doc's datas if we are merging 2 documents
		load(root);		// Only the document's Datas
	loadDoc(root);	// All the document's objects

	for(auto object : pandaObjects)
		object->reset();

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

	for(auto object : selectedObjects)
		object->reset();

	if(bSelected || !selectedObjects.empty())
	{
		emit selectionChanged();
		emit selectedObject(getCurrentSelectedObject());
	}

	return bVal;
}

bool PandaDocument::saveDoc(QDomDocument& doc, QDomElement& root, const ObjectsSelection& selected)
{
	typedef QPair<BaseData*, BaseData*> DataPair;
	QList<DataPair> links;

	typedef QPair<quint32, quint32> IntPair;
	QList<IntPair> dockedObjects;

	// Saving objects
	for(auto object : selected)
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
			if(parent && contains(selected, parent->getOwner()))
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
		auto object = createObject(registryName);
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
	emit selectedObject(nullptr);
	emit selectionChanged();

	for(auto object : pandaObjects)
		emit removedObject(object.data());

	pandaObjectsMap.clear();
	pandaObjects.clear();
	currentIndex = 1;
	animTime.setValue(0.0);
	timestep.setValue((PReal)0.01);
	useTimer.setValue(1);
	renderSize.setValue(Point(800,600));
	backgroundColor.setValue(Color::white());
	renderedImage.getAccessor()->clear();
	useMultithread.setValue(0);
	renderFrameBuffer.clear();

	animPlaying = false;
	animMultithread = false;
	animTimer->stop();
	if(m_scheduler)
		m_scheduler->stop();

	m_undoStack->clear();

	emit modified();
	emit timeChanged();
}

PandaObject* PandaDocument::createObject(QString registryName)
{
	auto object = ObjectFactory::getInstance()->create(registryName, this);
	if(object)
		addCommand(new AddObjectCommand(this, object));

	return object.data();
}

PandaDocument::ObjectPtr PandaDocument::getSharedPointer(PandaObject* object)
{
	auto iter = std::find(pandaObjects.begin(), pandaObjects.end(), object);
	if(iter != pandaObjects.end())
		return *iter;

	return ObjectPtr();
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

QSize PandaDocument::getRenderSize()
{
	Point pt = renderSize.getValue();
	return QSize(qMax<PReal>(1, floor(pt.x)), qMax<PReal>(1, floor(pt.y)));
}

void PandaDocument::setMouseClick(int state)
{
	if(mouseClickBuffer && !state) // Pressed & released in 1 timestep, we will send 2 events
		mouseClickBuffer = -1;
	else
		mouseClickBuffer = state;
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

void PandaDocument::selectionAdd(PandaObject* object)
{
	if(!contains(selectedObjects, object))
	{
		selectedObjects.append(object);
		emit selectedObject(object);
		emit selectionChanged();
	}
}

void PandaDocument::selectionRemove(PandaObject* object)
{
	if(contains(selectedObjects, object))
	{
		selectedObjects.removeAll(object);
		emit selectedObject(selectedObjects.back());
		emit selectionChanged();
	}
}

void PandaDocument::selectAll()
{
	selectedObjects.clear();
	for(auto object : pandaObjects)
		selectedObjects.push_back(object.data());
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

		auto currentSelected = selectedObjects.back();
		selectedObjects = closedList.toList();
		setCurrentSelectedObject(currentSelected);
		emit selectionChanged();
	}
}

void PandaDocument::addObject(ObjectPtr object)
{
	pandaObjectsMap.insert(object->getIndex(), object.data());
	pandaObjects.append(object);
	emit addedObject(object.data());
}

void PandaDocument::removeObject(PandaObject* object)
{
	emit removedObject(object);
	pandaObjectsMap.remove(object->getIndex());
	selectedObjects.removeAll(object);

	remove(pandaObjects, object);
	emit modified();
}

void PandaDocument::setDataDirty(BaseData* data)
{
	if(animMultithread && m_scheduler)
		m_scheduler->setDataDirty(data);
}

void PandaDocument::setDataReady(BaseData* data)
{
	if(animMultithread && m_scheduler)
		m_scheduler->setDataReady(data);
}

void PandaDocument::waitForOtherTasksToFinish(bool mainThread)
{
	if(animMultithread && m_scheduler)
		m_scheduler->waitForOtherTasks(mainThread);
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

void PandaDocument::onDirtyObject(PandaObject* object)
{
	emit dirtyObject(object);
	if(object == getCurrentSelectedObject())
		emit selectedObjectIsDirty(object);
	emit modified();
}

void PandaDocument::onModifiedObject(PandaObject* object)
{
	emit modifiedObject(object);
}

void PandaDocument::update()
{
	if(!renderFrameBuffer || renderFrameBuffer->size() != getRenderSize())
	{
		renderFrameBuffer.reset(new QOpenGLFramebufferObject(getRenderSize()));
		renderedImage.getAccessor()->setFbo(renderFrameBuffer);
	}

	helper::GradientCache::getInstance()->resetUsedFlag();
	helper::ShaderCache::getInstance()->resetUsedFlag();

	if(animMultithread && m_scheduler)
		m_scheduler->update();

	defaultLayer->updateIfDirty();

	for(auto obj : pandaObjects)
	{
		if(dynamic_cast<BaseLayer*>(obj.data()))
			obj->updateIfDirty();
	}

	render();

	helper::GradientCache::getInstance()->clearUnused();
	helper::ShaderCache::getInstance()->clearUnused();
	cleanDirty();
}

const ImageWrapper& PandaDocument::getRenderedImage()
{
	updateIfDirty();
	return renderedImage.getValue();
}

QSharedPointer<QOpenGLFramebufferObject> PandaDocument::getFBO()
{
	updateIfDirty();
	return renderFrameBuffer;
}

void PandaDocument::render()
{
#ifdef PANDA_LOG_EVENTS
	{
		helper::ScopedEvent log1("prepareRender");
#endif

	renderFrameBuffer->bind();
	glViewport(0, 0, renderFrameBuffer->width(), renderFrameBuffer->height());

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, renderFrameBuffer->width(), renderFrameBuffer->height(), 0, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	Color col = backgroundColor.getValue();
	glClearColor(col.r, col.g, col.b, col.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef PANDA_LOG_EVENTS
	}
#endif

	defaultLayer->mergeLayer();

	for(auto obj : pandaObjects)
	{
		BaseLayer* layer = dynamic_cast<BaseLayer*>(obj.data());
		if(layer)
			layer->mergeLayer();
	}

#ifdef PANDA_LOG_EVENTS
	helper::ScopedEvent log2("release FBO");
#endif

	renderFrameBuffer->release();
}

void PandaDocument::moveLayerUp(PandaObject* layer)
{
	if(!layer)
		return;
	auto iter = std::find(pandaObjects.begin(), pandaObjects.end(), layer);
	if(iter == pandaObjects.end())
		return;
	auto object = *iter; // Get the QSharedPointer
	int index = iter - pandaObjects.begin();
	int nb = pandaObjects.size();
	for(++index;index<nb;++index)
	{
		BaseLayer* otherLayer = dynamic_cast<BaseLayer*>(pandaObjects[index].data());
		if(otherLayer)
		{
			remove(pandaObjects, layer);
			pandaObjects.insert(index, object);
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
	auto iter = std::find(pandaObjects.begin(), pandaObjects.end(), layer);
	if(iter == pandaObjects.end())
		return;
	auto object = *iter; // Get the QSharedPointer
	int index = iter - pandaObjects.begin();
	for(--index;index>=0;--index)
	{
		BaseLayer* otherLayer = dynamic_cast<BaseLayer*>(pandaObjects[index].data());
		if(otherLayer)
		{
			remove(pandaObjects, layer);
			pandaObjects.insert(index, object);
			setDirtyValue();
			emit modified();
			return;
		}
	}
}

void PandaDocument::setDirtyValue()
{
	PandaObject::setDirtyValue();
	if(!isInStep && !getCurrentSelectedObject())
		emit selectedObjectIsDirty(this);
}

void PandaDocument::play(bool playing)
{
	animPlaying = playing;
	if(animPlaying)
	{
		animMultithread = useMultithread.getValue();
		if(animMultithread)
		{
			if(!m_scheduler)
				m_scheduler.reset(new Scheduler(this));
			m_scheduler->init();
		}
#ifdef PANDA_LOG_EVENTS
		else
		{
			helper::UpdateLogger::getInstance()->setNbThreads(1);
			helper::UpdateLogger::getInstance()->setupThread(0);
		}
#endif
		if(useTimer.getValue())
			animTimer->start(qMax((PReal)0.0, timestep.getValue() * 1000));
		else
			animTimer->start(0);
	}
	else
	{
		animTimer->stop();
		if(animMultithread && m_scheduler)
			m_scheduler->stop();
		animMultithread = false;
	}
}

void PandaDocument::step()
{
	QElapsedTimer durationTimer;
	durationTimer.start();
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->startLog(this);
#endif

	isInStep = true;
	for(auto object : pandaObjects)
		object->beginStep();

	if(animPlaying && animMultithread && m_scheduler)
		m_scheduler->setDirty();

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

	isInStep = false;
	for(auto object : pandaObjects)
		object->endStep();

#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->stopLog();
#endif

	unsigned int lastFrameDuration = durationTimer.elapsed();
	emit timeChanged();

	const auto obj = getCurrentSelectedObject();
	if(obj)
		emit selectedObjectIsDirty(obj);
	else
		emit selectedObjectIsDirty(this);
	emit modified();

	if(animPlaying && useTimer.getValue())	// Restart the timer taking into consideration the time it took to render this frame
		animTimer->start(qMax((PReal)0.0, timestep.getValue() * 1000 - lastFrameDuration - 1));
}

void PandaDocument::rewind()
{
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->startLog(this);
#endif
	renderFrameBuffer.reset();
	animTime.setValue(0.0);
	mousePosition.setValue(mousePositionBuffer);
	mouseClick.setValue(0);
	for(auto object : pandaObjects)
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
	auto object = createObject(registryName);

	BaseData* inputData = object->getData("input");
	if(inputData)
	{
		inputData->copyValueFrom(data);
		inputData->setWidget(data->getWidget());
		inputData->setWidgetData(data->getWidgetData());
	}
}

void PandaDocument::addCommand(QUndoCommand* command)
{
	m_undoStack->push(command);
}

ScopedMacro PandaDocument::beginCommandMacro(QString text)
{
	m_undoStack->beginMacro(text);
	++m_inCommandMacro;
	return ScopedMacro(this);
}

void PandaDocument::endCommandMacro()
{
	--m_inCommandMacro;
	m_undoStack->endMacro();
}

void PandaDocument::createUndoRedoActions(QObject* parent, QAction*& undoAction, QAction*& redoAction)
{
	undoAction = m_undoStack->createUndoAction(parent);
	redoAction = m_undoStack->createRedoAction(parent);
}

void PandaDocument::clearCommands()
{
	m_undoStack->clear();
}

} // namespace panda


