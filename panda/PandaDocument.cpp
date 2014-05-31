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

#include <QtWidgets>
#include <QOpenGLFramebufferObject>
#include <QElapsedTimer>
#include <QUndoStack>
#include <QDomDocument>
#include <QOpenGLFunctions>

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
	, m_currentIndex(1)
	, m_renderSize(initData(&m_renderSize, Point(800,600), "render size", "Size of the image to be rendered"))
	, m_backgroundColor(initData(&m_backgroundColor, Color::white(), "background color", "Background color of the image to be rendered"))
	, m_animTime(initData(&m_animTime, (PReal)0.0, "time", "Time of the animation"))
	, m_timestep(initData(&m_timestep, (PReal)0.01, "timestep", "Time step of the animation"))
	, m_useTimer(initData(&m_useTimer, 1, "use timer", "If true, wait before the next timestep. If false, compute the next one as soon as the previous finished."))
	, m_mousePosition(initData(&m_mousePosition, "mouse position", "Current position of the mouse in the render view"))
	, m_mouseClick(initData(&m_mouseClick, 0, "mouse click", "1 if the left mouse button is pressed"))
	, m_renderedImage(initData(&m_renderedImage, "rendered image", "Current image displayed"))
	, m_useMultithread(initData(&m_useMultithread, 0, "use multithread", "Optimize computation for multiple CPU cores"))
	, m_mouseClickBuffer(0)
	, m_animPlaying(false)
	, m_animMultithread(false)
	, m_currentCommand(nullptr)
	, m_inCommandMacro(0)
	, m_resetting(false)
{
	addInput(&m_renderSize);
	addInput(&m_backgroundColor);
	addInput(&m_timestep);
	addInput(&m_useTimer);
	addInput(&m_useMultithread);

	m_useTimer.setWidget("checkbox");

	// Not connecting to the document, otherwise it would update the layers each time we get the time.
	m_animTime.setOutput(true);
	m_animTime.setReadOnly(true);

	m_mousePosition.setOutput(true);
	m_mousePosition.setReadOnly(true);

	m_mouseClick.setOutput(true);
	m_mouseClick.setReadOnly(true);
	m_mouseClick.setWidget("checkbox");

	m_useMultithread.setWidget("checkbox");

	connect(this, SIGNAL(modifiedObject(panda::PandaObject*)), this, SIGNAL(modified()));
	connect(this, SIGNAL(addedObject(panda::PandaObject*)), this, SIGNAL(modified()));

	m_defaultLayer = new Layer(this);
	m_defaultLayer->setInternalData("Default Layer", 0);
	m_defaultLayer->getLayerNameData().setValue("Default Layer");

	setInternalData("Document", 0);

	m_animTimer = new QTimer(this);
	connect(m_animTimer, SIGNAL(timeout()), this, SLOT(step()));

	m_undoStack = new QUndoStack(this);
	m_undoStack->setUndoLimit(25);

	m_parentDocument = this;
}

PandaDocument::~PandaDocument()
{
	if(m_scheduler)
		m_scheduler->stop();

	m_resetting = true;

	// Just to be sure everything goes smoothly
	for(auto object : m_objects)
		object->preDestruction();

	m_objects.clear();
	m_undoStack->clear();
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
	for(auto object : m_objects)
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

	m_selectedObjects.clear();
	QDomElement root = doc.documentElement();
	if(!isImport)	// Bugfix: don't read the doc's datas if we are merging 2 documents
		load(root);		// Only the document's Datas
	loadDoc(root);	// All the document's objects

	for(auto object : m_objects)
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

	saveDoc(doc, root, m_selectedObjects);

	return doc.toString(4);
}

bool PandaDocument::readTextDocument(QString& text)
{
	QDomDocument doc("Panda");
	if(!doc.setContent(text))
		return false;

	bool bSelected = !m_selectedObjects.isEmpty();
	m_selectedObjects.clear();

	QDomElement root = doc.documentElement();
	bool bVal = loadDoc(root);

	for(auto object : m_selectedObjects)
		object->reset();

	if(bSelected || !m_selectedObjects.empty())
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
				links.push_back(qMakePair(data, parent));
		}

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			for(auto dockable : dock->getDockables())
				dockedObjects.push_back(qMakePair(dock->getIndex(), dockable->getIndex()));
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
	auto factory = ObjectFactory::getInstance();

	// Loading objects
	QDomElement elem = root.firstChildElement("Object");
	while(!elem.isNull())
	{
		QString registryName = elem.attribute("type");
		if(registryName.isEmpty())
			return false;
		quint32 index = elem.attribute("index").toUInt();
		auto object = factory->create(registryName, this);
		if(object)
		{
			addObject(object);
			importIndicesMap[index] = object->getIndex();
			m_selectedObjects.push_back(object.data());

			object->load(elem);

			emit loadingObject(elem, object.data());
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
	m_resetting = true;

	m_selectedObjects.clear();
	emit selectedObject(nullptr);
	emit selectionChanged();

	for(auto object : m_objects)
	{
		emit removedObject(object.data());
		object->preDestruction();
	}

	m_objects.clear();
	m_currentIndex = 1;
	m_animTime.setValue(0.0);
	m_timestep.setValue((PReal)0.01);
	m_useTimer.setValue(1);
	m_renderSize.setValue(Point(800,600));
	m_backgroundColor.setValue(Color::white());
	m_renderedImage.getAccessor()->clear();
	m_useMultithread.setValue(0);
	m_renderFBO.clear();

	m_animPlaying = false;
	m_animMultithread = false;
	m_animTimer->stop();
	if(m_scheduler)
		m_scheduler->stop();

	m_undoStack->clear();

	helper::GradientCache::getInstance()->clear();
	helper::ShaderCache::getInstance()->clear();

	emit modified();
	emit timeChanged();

	m_resetting = false;
}

PandaDocument::ObjectPtr PandaDocument::getSharedPointer(PandaObject* object)
{
	auto iter = std::find(m_objects.begin(), m_objects.end(), object);
	if(iter != m_objects.end())
		return *iter;

	return ObjectPtr();
}

PandaObject* PandaDocument::getCurrentSelectedObject()
{
	if(m_selectedObjects.empty())
		return nullptr;
	else
		return m_selectedObjects.back();
}

void PandaDocument::setCurrentSelectedObject(PandaObject* object)
{
	m_selectedObjects.removeAll(object);
	m_selectedObjects.push_back(object);
	emit selectedObject(object);
	emit selectionChanged();
}

QSize PandaDocument::getRenderSize()
{
	Point pt = m_renderSize.getValue();
	return QSize(qMax<PReal>(1, floor(pt.x)), qMax<PReal>(1, floor(pt.y)));
}

void PandaDocument::setMouseClick(int state)
{
	if(m_mouseClickBuffer && !state) // Pressed & released in 1 timestep, we will send 2 events
		m_mouseClickBuffer = -1;
	else
		m_mouseClickBuffer = state;
}

void PandaDocument::copy()
{
	if(m_selectedObjects.isEmpty())
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
	if(!contains(m_selectedObjects, object))
	{
		m_selectedObjects.push_back(object);
		emit selectedObject(object);
		emit selectionChanged();
	}
}

void PandaDocument::selectionRemove(PandaObject* object)
{
	if(contains(m_selectedObjects, object))
	{
		m_selectedObjects.removeAll(object);
		if(m_selectedObjects.empty())
			emit selectedObject(nullptr);
		else
			emit selectedObject(m_selectedObjects.back());
		emit selectionChanged();
	}
}

void PandaDocument::selectAll()
{
	m_selectedObjects.clear();
	for(auto object : m_objects)
		m_selectedObjects.push_back(object.data());
	emit selectedObject(m_selectedObjects.back());
	emit selectionChanged();
}

void PandaDocument::selectNone()
{
	if(!m_selectedObjects.empty())
	{
		m_selectedObjects.clear();
		emit selectedObject(nullptr);
		emit selectionChanged();
	}
}

void PandaDocument::selectConnected()
{
	if(!m_selectedObjects.empty())
	{
		QSet<PandaObject*> closedList, openList;
		openList = QSet<PandaObject*>::fromList(m_selectedObjects);
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
				if(dock != m_defaultLayer && !closedList.contains(dock))
					openList.insert(dock);
			}

			DockObject* dock = dynamic_cast<DockObject*>(object);
			if(dock)
			{
				for(auto dockable : dock->getDockables())
				{
					if(!closedList.contains(dockable))
						openList.insert(dockable);
				}
			}
		}

		auto currentSelected = m_selectedObjects.back();
		m_selectedObjects = closedList.toList();
		setCurrentSelectedObject(currentSelected);
		emit selectionChanged();
	}
}

void PandaDocument::addObject(ObjectPtr object)
{
	m_objects.push_back(object);
	object->addedToDocument();
	emit addedObject(object.data());
}

void PandaDocument::removeObject(PandaObject* object)
{
	object->removedFromDocument();
	emit removedObject(object);
	remove(m_objects, object);

	selectionRemove(object);
	emit modified();
}

void PandaDocument::setDataDirty(BaseData* data)
{
	if(m_animMultithread && m_scheduler)
		m_scheduler->setDataDirty(data);
}

void PandaDocument::setDataReady(BaseData* data)
{
	if(m_animMultithread && m_scheduler)
		m_scheduler->setDataReady(data);
}

void PandaDocument::waitForOtherTasksToFinish(bool mainThread)
{
	if(m_animMultithread && m_scheduler)
		m_scheduler->waitForOtherTasks(mainThread);
}

PandaObject* PandaDocument::findObject(quint32 objectIndex)
{
	auto iter = std::find_if(m_objects.cbegin(), m_objects.cend(), [objectIndex](const ObjectPtr& object){
		return object->getIndex() == objectIndex;
	});

	if(iter != m_objects.end())
		return iter->data();

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
	if(m_resetting)
		return;

	emit dirtyObject(object);
	if(object == getCurrentSelectedObject())
		emit selectedObjectIsDirty(object);
	emit modified();
}

void PandaDocument::onModifiedObject(PandaObject* object)
{
	if(m_resetting)
		return;

	emit modifiedObject(object);
}

void PandaDocument::update()
{
	if(!m_renderFBO || m_renderFBO->size() != getRenderSize())
	{
		m_renderFBO.reset(new QOpenGLFramebufferObject(getRenderSize()));
		m_secondRenderFBO.reset(new QOpenGLFramebufferObject(getRenderSize()));
		m_renderedImage.getAccessor()->setFbo(m_renderFBO);
	}

	if(!m_mergeLayersShader)
	{
		m_mergeLayersShader.reset(new QOpenGLShaderProgram());
		m_mergeLayersShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/mergeLayers.v.glsl");
		m_mergeLayersShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/mergeLayers.f.glsl");
		m_mergeLayersShader->link();
		m_mergeLayersShader->bind();

		m_mergeLayersShader->setUniformValue("texS", 0);
		m_mergeLayersShader->setUniformValue("texD", 1);

		m_mergeLayersShader->release();
	}

	helper::GradientCache::getInstance()->resetUsedFlag();
//	helper::ShaderCache::getInstance()->resetUsedFlag();

	if(m_animMultithread && m_scheduler)
		m_scheduler->update();

	m_defaultLayer->updateIfDirty();

	for(auto obj : m_objects)
	{
		if(dynamic_cast<BaseLayer*>(obj.data()))
			obj->updateIfDirty();
	}

	render();

	helper::GradientCache::getInstance()->clearUnused();
//	helper::ShaderCache::getInstance()->clearUnused();
	cleanDirty();
}

const ImageWrapper& PandaDocument::getRenderedImage()
{
	updateIfDirty();
	return m_renderedImage.getValue();
}

QSharedPointer<QOpenGLFramebufferObject> PandaDocument::getFBO()
{
	updateIfDirty();
	return m_renderFBO;
}

void PandaDocument::render()
{
	GLfloat w = m_renderFBO->width(), h = m_renderFBO->height();
#ifdef PANDA_LOG_EVENTS
	{
		helper::ScopedEvent log1("prepareRender");
#endif

	m_secondRenderFBO->bind();
	glViewport(0, 0, w, h);
	Color col = m_backgroundColor.getValue();
	glClearColor(col.r, col.g, col.b, col.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_secondRenderFBO->release();

	m_renderFBO->bind();
	glViewport(0, 0, w, h);

#ifdef PANDA_LOG_EVENTS
	}
#endif

	m_mergeLayersShader->bind();

	QMatrix4x4 mvp;
	mvp.ortho(0, w, h, 0, -10, 10);
	m_mergeLayersShader->setUniformValue("MVP", mvp);

	QOpenGLFunctions glFunctions(QOpenGLContext::currentContext());

	GLfloat verts[8], texCoords[8];

	verts[2*2+0] = w; verts[2*2+1] = 0;
	verts[3*2+0] = 0; verts[3*2+1] = 0;
	verts[1*2+0] = 0; verts[1*2+1] = h;
	verts[0*2+0] = w; verts[0*2+1] = h;

	texCoords[0*2+0] = 1; texCoords[0*2+1] = 0;
	texCoords[1*2+0] = 0; texCoords[1*2+1] = 0;
	texCoords[3*2+0] = 0; texCoords[3*2+1] = 1;
	texCoords[2*2+0] = 1; texCoords[2*2+1] = 1;

	glEnable(GL_TEXTURE_2D);
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, verts );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, 0, texCoords );

#ifdef PANDA_LOG_EVENTS
	{
	helper::ScopedEvent log("merge default Layer");
#endif

	m_mergeLayersShader->setUniformValue("opacity", 1.0f);
	m_mergeLayersShader->setUniformValue("mode", 0);
	glFunctions.glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_secondRenderFBO->texture());
	glFunctions.glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_defaultLayer->getTextureId());
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

	m_renderFBO->release();

#ifdef PANDA_LOG_EVENTS
	}
#endif

	bool inverse = false;
	for(auto obj : m_objects)
	{
		BaseLayer* layer = dynamic_cast<BaseLayer*>(obj.data());
		if(layer)
		{
			float opacity = (float)layer->getOpacity();
			if(!opacity)
				continue;

#ifdef PANDA_LOG_EVENTS
			helper::ScopedEvent log2("merge Layer");
#endif

			m_mergeLayersShader->setUniformValue("opacity", opacity);
			m_mergeLayersShader->setUniformValue("mode", layer->getCompositionMode());

			inverse = !inverse;
			if(inverse)
			{
				m_secondRenderFBO->bind();
				glFunctions.glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_renderFBO->texture());
			}
			else
			{
				m_renderFBO->bind();
				glFunctions.glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_secondRenderFBO->texture());
			}

			glFunctions.glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, layer->getTextureId());
			glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

			if(inverse)
				m_secondRenderFBO->release();
			else
				m_renderFBO->release();
		}
	}

	glDisable(GL_TEXTURE_2D);

	m_mergeLayersShader->release();

	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );

	if(inverse)
	{
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log3("blit FBO");
#endif
		QOpenGLFramebufferObject::blitFramebuffer(m_renderFBO.data(), m_secondRenderFBO.data());
	}
}

void PandaDocument::moveLayerUp(PandaObject* layer)
{
	if(!layer)
		return;
	auto iter = std::find(m_objects.begin(), m_objects.end(), layer);
	if(iter == m_objects.end())
		return;
	auto object = *iter; // Get the QSharedPointer
	int index = iter - m_objects.begin();
	int nb = m_objects.size();
	for(++index;index<nb;++index)
	{
		BaseLayer* otherLayer = dynamic_cast<BaseLayer*>(m_objects[index].data());
		if(otherLayer)
		{
			remove(m_objects, layer);
			m_objects.insert(index, object);
			setDirtyValue(this);
			emit modified();
			return;
		}
	}
}

void PandaDocument::moveLayerDown(PandaObject *layer)
{
	if(!layer)
		return;
	auto iter = std::find(m_objects.begin(), m_objects.end(), layer);
	if(iter == m_objects.end())
		return;
	auto object = *iter; // Get the QSharedPointer
	int index = iter - m_objects.begin();
	for(--index;index>=0;--index)
	{
		BaseLayer* otherLayer = dynamic_cast<BaseLayer*>(m_objects[index].data());
		if(otherLayer)
		{
			remove(m_objects, layer);
			m_objects.insert(index, object);
			setDirtyValue(this);
			emit modified();
			return;
		}
	}
}

void PandaDocument::setDirtyValue(const DataNode* caller)
{
	PandaObject::setDirtyValue(caller);
	if(!m_isInStep && !getCurrentSelectedObject())
		emit selectedObjectIsDirty(this);
}

void PandaDocument::play(bool playing)
{
	m_animPlaying = playing;
	if(m_animPlaying)
	{
		m_animMultithread = m_useMultithread.getValue();
		if(m_animMultithread)
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
		if(m_useTimer.getValue())
			m_animTimer->start(qMax((PReal)0.0, m_timestep.getValue() * 1000));
		else
			m_animTimer->start(0);
	}
	else
	{
		m_animTimer->stop();
		if(m_animMultithread && m_scheduler)
			m_scheduler->stop();
		m_animMultithread = false;
	}
}

void PandaDocument::step()
{
	QElapsedTimer durationTimer;
	durationTimer.start();
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->startLog(this);
#endif

	m_isInStep = true;
	for(auto object : m_objects)
		object->beginStep();

	if(m_animPlaying && m_animMultithread && m_scheduler)
		m_scheduler->setDirty();

	m_animTime.setValue(m_animTime.getValue() + m_timestep.getValue());
	m_mousePosition.setValue(m_mousePositionBuffer);
	if(m_mouseClickBuffer < 0)
	{
		m_mouseClick.setValue(1);
		m_mouseClickBuffer = 0;
	}
	else
		m_mouseClick.setValue(m_mouseClickBuffer);
	setDirtyValue(this);
	updateIfDirty();

	m_isInStep = false;
	for(auto object : m_objects)
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

	if(m_animPlaying && m_useTimer.getValue())	// Restart the timer taking into consideration the time it took to render this frame
		m_animTimer->start(qMax((PReal)0.0, m_timestep.getValue() * 1000 - lastFrameDuration - 1));
}

void PandaDocument::rewind()
{
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->startLog(this);
#endif
	m_renderFBO.reset();
	m_animTime.setValue(0.0);
	m_mousePosition.setValue(m_mousePositionBuffer);
	m_mouseClick.setValue(0);
	for(auto object : m_objects)
		object->reset();
	setDirtyValue(this);
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
	auto object = ObjectFactory::getInstance()->create(registryName, this);
	if(!object)
		return;

	addObject(object);
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
	if(m_resetting)
	{
		delete command;
		return;
	}

	auto oldCommand = m_currentCommand;
	m_currentCommand = command;
	m_undoStack->push(command);
	m_currentCommand = oldCommand;
}

QSharedPointer<ScopedMacro> PandaDocument::beginCommandMacro(QString text)
{
	m_undoStack->beginMacro(text);
	++m_inCommandMacro;
	return QSharedPointer<ScopedMacro>(new ScopedMacro(this));
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


