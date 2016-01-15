#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/Messaging.h>
#include <panda/Scheduler.h>
#include <panda/SimpleGUI.h>
#include <panda/TimedFunctions.h>
#include <panda/data/DataFactory.h>
#include <panda/object/Layer.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/GradientCache.h>
#include <panda/helper/ShaderCache.h>
#include <panda/helper/system/FileRepository.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/UpdateLogger.h>
#endif

#include <chrono>
#include <set>

namespace
{

int loadGlew()
{
	// Get OpenGL functions
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

	return 1;
}

long long currentTime()
{
	using namespace std::chrono;
	auto now = time_point_cast<microseconds>(high_resolution_clock::now());
	return now.time_since_epoch().count();
}

double toSeconds(long long elapsed)
{
	using namespace std::chrono;
	microseconds durMicro(elapsed);
	auto durSec = duration_cast<seconds>(durMicro);
	return duration_cast<duration<double>>(durSec).count();
}

}

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Point;

PandaDocument::PandaDocument(QObject* parent, gui::BaseGUI& gui)
	: PandaObject(nullptr)
	, QObject(parent)
	, m_currentIndex(1)
	, m_renderSize(initData(Point(800,600), "render size", "Size of the image to be rendered"))
	, m_backgroundColor(initData(Color::white(), "background color", "Background color of the image to be rendered"))
	, m_animTime(initData(0.0, "time", "Time of the animation"))
	, m_timestep(initData(0.01, "timestep", "Time step of the animation"))
	, m_useTimer(initData(1, "use timer", "If true, wait before the next timestep. If false, compute the next one as soon as the previous finished."))
	, m_mousePosition(initData("mouse position", "Current position of the mouse in the render view"))
	, m_mouseClick(initData(0, "mouse click", "1 if the left mouse button is pressed"))
	, m_renderedImage(initData("rendered image", "Current image displayed"))
	, m_useMultithread(initData(0, "use multithread", "Optimize computation for multiple CPU cores"))
	, m_mouseClickBuffer(0)
	, m_inCommandMacro(0)
	, m_resetting(false)
	, m_isGLInitialized(false)
	, m_animPlaying(false)
	, m_animMultithread(false)
	, m_iNbFrames(0)
	, m_currentFPS(0)
	, m_gui(gui)
{
	addInput(m_renderSize);
	addInput(m_backgroundColor);
	addInput(m_timestep);
	addInput(m_useTimer);
	addInput(m_useMultithread);

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

	m_defaultLayer = new Layer(this);
	m_defaultLayer->getLayerNameData().setValue("Default Layer");

	setInternalData("Document", 0);

	m_undoStack.setUndoLimit(25);

	m_parentDocument = this;
}

PandaDocument::~PandaDocument()
{
	if(m_scheduler)
		m_scheduler->stop();

	TimedFunctions::instance().shutdown();

	m_resetting = true;

	// Just to be sure everything goes smoothly
	for(auto object : m_objects)
		object->preDestruction();

	m_objects.clear();
	m_undoStack.clear();
}

bool PandaDocument::writeFile(const std::string& fileName)
{
	XmlDocument doc;
	auto root = doc.root();
	root.setName("Panda");
	save(root);	// The document's Datas
	ObjectsSelection allObjects;
	for(auto object : m_objects)
		allObjects.push_back(object.get());
	saveDoc(root, allObjects);	// The document and all of its objects

	bool result = doc.saveToFile(fileName);
	if (!result)
		m_gui.messageBox(gui::MessageBoxType::warning, "Panda", "Cannot write file " + fileName);

	return result;
}

bool PandaDocument::readFile(const std::string& fileName, bool isImport)
{
	XmlDocument doc;
	if (!doc.loadFromFile(fileName))
	{
		m_gui.messageBox(gui::MessageBoxType::warning, "Panda", "Cannot parse xml file  " + fileName + ".");
		return false;
	}

	m_selectedObjects.clear();
	auto root = doc.root();
	if(!isImport)	// Bugfix: don't read the doc's datas if we are merging 2 documents
		load(root);		// Only the document's Datas
	loadDoc(root);	// All the document's objects

	for(auto object : m_objects)
		object->reset();

	m_selectionChangedSignal.run();
	m_selectedObjectSignal.run(getCurrentSelectedObject());

	return true;
}

std::string PandaDocument::writeTextDocument()
{
	XmlDocument doc;
	auto root = doc.root();
	root.setName("Panda");

	saveDoc(root, m_selectedObjects);

	return doc.saveToMemory();
}

bool PandaDocument::readTextDocument(const std::string& text)
{
	XmlDocument doc;
	if (!doc.loadFromMemory(text))
		return false;

	bool bSelected = !m_selectedObjects.empty();
	m_selectedObjects.clear();

	auto root = doc.root();
	bool bVal = loadDoc(root);

	for(auto object : m_selectedObjects)
		object->reset();

	if(bSelected || !m_selectedObjects.empty())
	{
		m_selectionChangedSignal.run();
		m_selectedObjectSignal.run(getCurrentSelectedObject());
	}

	return bVal;
}

bool PandaDocument::saveDoc(XmlElement& root, const ObjectsSelection& selected)
{
	typedef std::pair<BaseData*, BaseData*> DataPair;
	std::vector<DataPair> links;

	typedef std::pair<quint32, quint32> IntPair;
	std::vector<IntPair> dockedObjects;

	// Saving objects
	for(auto object : selected)
	{
		auto elem = root.addChild("Object");
		elem.setAttribute("type", ObjectFactory::getRegistryName(object));
		elem.setAttribute("index", object->getIndex());

		object->save(elem, &selected);

		// Preparing links
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* parent = data->getParent();
			if(parent && helper::contains(selected, parent->getOwner()))
				links.push_back(std::make_pair(data, parent));
		}

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			for(auto dockable : dock->getDockedObjects())
				dockedObjects.push_back(std::make_pair(dock->getIndex(), dockable->getIndex()));
		}

		m_savingObjectSignal.run(elem, object);
	}

	// Saving links
	for(const auto& link : links)
	{
		auto elem = root.addChild("Link");
		elem.setAttribute("object1", link.first->getOwner()->getIndex());
		elem.setAttribute("data1", link.first->getName());
		elem.setAttribute("object2", link.second->getOwner()->getIndex());
		elem.setAttribute("data2", link.second->getName());
	}

	// Saving docked objects list
	for(const auto& dockable : dockedObjects)
	{
		auto elem = root.addChild("Dock");
		elem.setAttribute("dock", dockable.first);
		elem.setAttribute("docked", dockable.second);
	}

	return true;
}

bool PandaDocument::loadDoc(XmlElement& root)
{
	m_startLoadingSignal.run();
	std::map<quint32, quint32> importIndicesMap;
	auto factory = ObjectFactory::getInstance();

	// Loading objects
	auto elem = root.firstChild("Object");
	while(elem)
	{
		std::string registryName = elem.attribute("type").toString();
		if(registryName.empty())
			return false;
		quint32 index = elem.attribute("index").toUnsigned();
		auto object = factory->create(registryName, this);
		if(object)
		{
			addObject(object);
			importIndicesMap[index] = object->getIndex();
			m_selectedObjects.push_back(object.get());

			object->load(elem);

			m_loadingObjectSignal.run(elem, object.get());
		}
		else
		{
			m_gui.messageBox(gui::MessageBoxType::warning, "Panda", "Could not create the object " + registryName + ".\nA plugin must be missing.");
			return false;
		}

		elem = elem.nextSibling("Object");
	}

	// Create links
	elem = root.firstChild("Link");
	while(elem)
	{
		quint32 index1, index2;
		std::string name1, name2;
		index1 = elem.attribute("object1").toUnsigned();
		index2 = elem.attribute("object2").toUnsigned();
		index1 = importIndicesMap[index1];
		index2 = importIndicesMap[index2];

		name1 = elem.attribute("data1").toString();
		name2 = elem.attribute("data2").toString();

		BaseData *data1, *data2;
		data1 = findData(index1, name1);
		data2 = findData(index2, name2);
		if(data1 && data2)
			data1->setParent(data2);

		elem = elem.nextSibling("Link");
	}

	// Put dockables in their docks
	elem = root.firstChild("Dock");
	while(elem)
	{
		quint32 dockIndex, dockableIndex;
		dockIndex = elem.attribute("dock").toUnsigned();
		dockableIndex = elem.attribute("docked").toUnsigned();
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

		elem = elem.nextSibling("Dock");
	}

	m_loadingFinishedSignal.run(); // For example if the view wants to do some computation

	return true;
}

void PandaDocument::resetDocument()
{
	m_resetting = true;

	m_selectedObjects.clear();
	m_selectedObjectSignal.run(nullptr);
	m_selectionChangedSignal.run();

	for(auto object : m_objects)
	{
		m_removedObjectSignal.run(object.get());
		object->preDestruction();
	}

	m_objects.clear();
	m_currentIndex = 1;
	m_animTimeVal = 0.0;
	m_animTime.setValue(0.0);
	m_timestep.setValue((PReal)0.01);
	m_mouseClickVal = 0;
	m_mouseClick.setValue(0);
	m_useTimer.setValue(1);
	m_renderSize.setValue(Point(800,600));
	m_backgroundColor.setValue(Color::white());
	m_renderedImage.getAccessor()->clear();
	m_useMultithread.setValue(0);
	m_renderFBO.reset();

	m_animPlaying = false;
	m_animMultithread = false;
	TimedFunctions::instance().cancelAll();
	if(m_scheduler)
		m_scheduler->stop();

	m_undoStack.clear();

	helper::GradientCache::getInstance()->clear();
	helper::ShaderCache::getInstance()->clear();

	m_modifiedSignal.run();
	m_timeChangedSignal.run();

	m_resetting = false;
}

PandaDocument::ObjectPtr PandaDocument::getSharedPointer(PandaObject* object) const
{
	auto iter = std::find_if(m_objects.begin(), m_objects.end(), [object](const ObjectPtr& ptr){
		return ptr.get() == object;
	});
	if(iter != m_objects.end())
		return *iter;

	return ObjectPtr();
}

int PandaDocument::getObjectPosition(PandaObject* object) const
{
	auto iter = std::find_if(m_objects.begin(), m_objects.end(), [object](const ObjectPtr& ptr){
		return ptr.get() == object;
	});
	if(iter != m_objects.end())
		return std::distance(m_objects.begin(), iter);

	return -1;
}

void PandaDocument::reinsertObject(PandaObject* object, int pos)
{
	auto iter = std::find_if(m_objects.begin(), m_objects.end(), [object](const ObjectPtr& ptr){
		return ptr.get() == object;
	});
	if(iter == m_objects.end())
		return;

	int oldPos = iter - m_objects.begin();
	ObjectPtr objectPtr = *iter;

	m_objects.erase(iter);
	if(pos > oldPos)
		--pos;

	m_objects.insert(m_objects.begin() + pos, objectPtr);

	setDirtyValue(this);
	m_reorderedObjectsSignal.run();
	m_modifiedSignal.run();
}

PandaObject* PandaDocument::getCurrentSelectedObject() const
{
	if(m_selectedObjects.empty())
		return nullptr;
	else
		return m_selectedObjects.back();
}

void PandaDocument::setCurrentSelectedObject(PandaObject* object)
{
	helper::removeAll(m_selectedObjects, object);
	m_selectedObjects.push_back(object);
	m_selectedObjectSignal.run(object);
	m_selectionChangedSignal.run();
}

graphics::Size PandaDocument::getRenderSize() const
{
	Point pt = m_renderSize.getValue();
	return graphics::Size(std::max<PReal>(1, floor(pt.x)), std::max<PReal>(1, floor(pt.y)));
}

void PandaDocument::setMouseClick(bool clicked, const types::Point& pos)
{
	if(m_mouseClickBuffer && !clicked) // Pressed & released in 1 timestep, we will send 2 events
		m_mouseClickBuffer = -1;
	else
		m_mouseClickBuffer = clicked;

	if (clicked)
		m_mousePressedSignal.run(pos);
	else
		m_mouseReleasedSignal.run(pos);
}

void PandaDocument::selectionAdd(PandaObject* object)
{
	if(!helper::contains(m_selectedObjects, object))
	{
		m_selectedObjects.push_back(object);
		m_selectedObjectSignal.run(object);
		m_selectionChangedSignal.run();
	}
}

void PandaDocument::selectionRemove(PandaObject* object)
{
	if(helper::contains(m_selectedObjects, object))
	{
		helper::removeAll(m_selectedObjects, object);
		if (m_selectedObjects.empty())
			m_selectedObjectSignal.run(nullptr);
		else
			m_selectedObjectSignal.run(m_selectedObjects.back());
		m_selectionChangedSignal.run();
	}
}

void PandaDocument::selectAll()
{
	m_selectedObjects.clear();
	for(auto object : m_objects)
		m_selectedObjects.push_back(object.get());
	m_selectedObjectSignal.run(m_selectedObjects.back());
	m_selectionChangedSignal.run();
}

void PandaDocument::selectNone()
{
	if(!m_selectedObjects.empty())
	{
		m_selectedObjects.clear();
		m_selectedObjectSignal.run(nullptr);
		m_selectionChangedSignal.run();
	}
}

void PandaDocument::selectConnected()
{
	if(!m_selectedObjects.empty())
	{
		std::set<PandaObject*> closedList, openList;
		openList.insert(m_selectedObjects.begin(), m_selectedObjects.end());
		while(!openList.empty())
		{
			PandaObject* object = *openList.begin();
			openList.erase(object);
			closedList.insert(object);

			for(BaseData* data : object->getInputDatas())
			{
				if(data->getParent())
				{
					PandaObject* connected = data->getParent()->getOwner();
					if(!closedList.count(connected))
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
						if (!closedList.count(connected))
							openList.insert(connected);
					}
				}
			}

			DockableObject* dockable = dynamic_cast<DockableObject*>(object);
			if(dockable)
			{
				PandaObject* dock = dockable->getParentDock();
				if (dock != m_defaultLayer && !closedList.count(dock))
					openList.insert(dock);
			}

			DockObject* dock = dynamic_cast<DockObject*>(object);
			if(dock)
			{
				for(auto dockable : dock->getDockedObjects())
				{
					if (!closedList.count(dockable))
						openList.insert(dockable);
				}
			}
		}

		auto currentSelected = m_selectedObjects.back();
		m_selectedObjects.assign(closedList.begin(), closedList.end());
		setCurrentSelectedObject(currentSelected);
		m_selectionChangedSignal.run();
	}
}

void PandaDocument::addObject(ObjectPtr object)
{
	m_objects.push_back(object);
	object->addedToDocument();
	m_addedObjectSignal.run(object.get());
	m_modifiedSignal.run();
}

void PandaDocument::removeObject(PandaObject* object)
{
	object->removedFromDocument();
	m_removedObjectSignal.run(object);
	helper::removeIf(m_objects, [object](const ObjectPtr& ptr){
		return ptr.get() == object;
	});

	selectionRemove(object);
	m_modifiedSignal.run();
}

void PandaDocument::setDataDirty(BaseData* data) const
{
	if(m_animMultithread && m_scheduler)
		m_scheduler->setDataDirty(data);
}

void PandaDocument::setDataReady(BaseData* data) const
{
	if(m_animMultithread && m_scheduler)
		m_scheduler->setDataReady(data);
}

void PandaDocument::waitForOtherTasksToFinish(bool mainThread) const
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
		return iter->get();

	return nullptr;
}

BaseData* PandaDocument::findData(quint32 objectIndex, const std::string& dataName)
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

	m_dirtyObjectSignal.run(object);
	if(object == getCurrentSelectedObject())
		m_selectedObjectIsDirtySignal.run(object);
	m_modifiedSignal.run();
}

void PandaDocument::onModifiedObject(PandaObject* object)
{
	if(m_resetting)
		return;

	m_modifiedObjectSignal.run(object);
	m_modifiedSignal.run();
}

void PandaDocument::onChangedDock(DockableObject* dockable)
{
	if(m_resetting)
		return;

	m_changedDockSignal.run(dockable);
}

void PandaDocument::update()
{
	static const int loadGlewVal = loadGlew();

	auto renderSize = getRenderSize();
	if(!m_renderFBO || m_renderFBO->size() != renderSize)
	{
		m_renderFBO = std::make_shared<graphics::Framebuffer>(renderSize);
		m_secondRenderFBO = std::make_shared<graphics::Framebuffer>(renderSize);
		m_renderedImage.getAccessor()->setFbo(*m_renderFBO);
	}

	if(!m_mergeLayersShader)
	{
		m_mergeLayersShader = std::make_shared<graphics::ShaderProgram>();
		m_mergeLayersShader->addShaderFromMemory(graphics::ShaderType::Vertex,
			helper::system::DataRepository.loadFile("shaders/mergeLayers.v.glsl"));
		m_mergeLayersShader->addShaderFromMemory(graphics::ShaderType::Fragment,
			helper::system::DataRepository.loadFile("shaders/mergeLayers.f.glsl"));
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
		if(dynamic_cast<BaseLayer*>(obj.get()))
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

std::shared_ptr<graphics::Framebuffer> PandaDocument::getFBO()
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

	glViewport(0, 0, w, h);
	Color col = m_backgroundColor.getValue();
	glClearColor(col.r, col.g, col.b, col.a);

	m_secondRenderFBO->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_secondRenderFBO->release();

	m_renderFBO->bind();

#ifdef PANDA_LOG_EVENTS
	}
#endif

	m_mergeLayersShader->bind();

	graphics::Mat4x4 mvp;
	mvp.ortho(0, w, h, 0, -10, 10);
	m_mergeLayersShader->setUniformValueMat4("MVP", mvp.data());

	GLfloat verts[8], texCoords[8];

	verts[2*2+0] = w; verts[2*2+1] = 0;
	verts[3*2+0] = 0; verts[3*2+1] = 0;
	verts[1*2+0] = 0; verts[1*2+1] = h;
	verts[0*2+0] = w; verts[0*2+1] = h;

	texCoords[0*2+0] = 1; texCoords[0*2+1] = 0;
	texCoords[1*2+0] = 0; texCoords[1*2+1] = 0;
	texCoords[3*2+0] = 0; texCoords[3*2+1] = 1;
	texCoords[2*2+0] = 1; texCoords[2*2+1] = 1;

#ifdef PANDA_LOG_EVENTS
	{
	helper::ScopedEvent log("merge default Layer");
#endif

	m_mergeLayersShader->enableAttributeArray("vertex");
	m_mergeLayersShader->setAttributeArray("vertex", verts, 2);

	m_mergeLayersShader->enableAttributeArray("texCoord");
	m_mergeLayersShader->setAttributeArray("texCoord", texCoords, 2);

	m_mergeLayersShader->setUniformValue("opacity", 1.0f);
	m_mergeLayersShader->setUniformValue("mode", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_defaultLayer->getTextureId());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_secondRenderFBO->texture());
	glActiveTexture(GL_TEXTURE0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	m_renderFBO->release();

#ifdef PANDA_LOG_EVENTS
	}
#endif

	bool inverse = false;
	for(auto obj : m_objects)
	{
		BaseLayer* layer = dynamic_cast<BaseLayer*>(obj.get());
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

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, layer->getTextureId());

			inverse = !inverse;
			if(inverse)
			{
				m_secondRenderFBO->bind();
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_renderFBO->texture());
			}
			else
			{
				m_renderFBO->bind();
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, m_secondRenderFBO->texture());
			}

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glActiveTexture(GL_TEXTURE0);

			if(inverse)
				m_secondRenderFBO->release();
			else
				m_renderFBO->release();
		}
	}

	m_mergeLayersShader->disableAttributeArray("vertex");
	m_mergeLayersShader->disableAttributeArray("texCoord");
	m_mergeLayersShader->release();

	if(inverse)
	{
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log3("blit FBO");
#endif
		graphics::Framebuffer::blitFramebuffer(*m_renderFBO, *m_secondRenderFBO);
	}
}

void PandaDocument::setDirtyValue(const DataNode* caller)
{
	PandaObject::setDirtyValue(caller);
	if(!m_isInStep && !getCurrentSelectedObject())
		m_selectedObjectIsDirtySignal.run(this);

	if(caller == &m_renderSize)
		m_renderSizeChangedSignal.run();
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

		m_gui.executeByUI([this]() { step(); });

		m_iNbFrames = 0;
		m_fpsTime = currentTime();
	}
	else
	{
		if(m_animMultithread && m_scheduler)
			m_scheduler->stop();
		m_animMultithread = false;
		TimedFunctions::instance().cancelRun(m_animFunctionIndex);
	}
}

void PandaDocument::step()
{
	auto startTime = std::chrono::high_resolution_clock::now();
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->startLog(this);
#endif

	m_isInStep = true;
	// Force the value of isInStep, because some objects will propagate dirtyValue during beginStep
	for(auto& object : m_objects)
		object->setInStep(true);

	// First update the value of the document (without modifying the corresponding Data)
	// This is so an object reacting on the time having changed can get the correct value of the mouse position (and not the one from the previous step)
	m_animTimeVal += m_timestep.getValue();
	m_mousePositionVal = m_mousePositionBuffer;
	if(m_mouseClickBuffer < 0)
	{
		m_mouseClickVal = 1;
		m_mouseClickBuffer = 0;
	}
	else
		m_mouseClickVal = m_mouseClickBuffer;

	// Let some objects set dirtyValue for each step
	for(auto& object : m_objects)
		object->beginStep();

	if(m_animPlaying && m_animMultithread && m_scheduler)
		m_scheduler->setDirty();

	// Update the documents Data (all the values are already correct if using the getters)
	m_animTime.setValue(m_animTimeVal);
	m_mousePosition.setValue(m_mousePositionVal);
	m_mouseClick.setValue(m_mouseClickVal);

	setDirtyValue(this);
	updateIfDirty();

	m_isInStep = false;
	for(auto& object : m_objects)
		object->endStep();

#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->stopLog();
#endif

	m_timeChangedSignal.run();

	const auto obj = getCurrentSelectedObject();
	if (obj)
		m_selectedObjectIsDirtySignal.run(obj);
	else
		m_selectedObjectIsDirtySignal.run(this);
	m_modifiedSignal.run();

	if (m_animPlaying)	
	{
		if (m_useTimer.getValue()) // Restart the timer taking into consideration the time it took to render this frame
		{
			auto endTime = std::chrono::high_resolution_clock::now();
			auto frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
			double delay = m_timestep.getValue() - frameDuration.count() / 1000.0 - 0.001; // How long do we wait until we start the next step ?
			TimedFunctions::instance().cancelRun(m_animFunctionIndex); // Remove previous
			m_animFunctionIndex = TimedFunctions::instance().delayRun(delay, [this]() { // Ask for the delayed execution of step()
				m_gui.executeByUI([this]() { step(); });  // But we want step() to be run on the GUI thread
			});
		}
		else
			m_gui.executeByUI([this]() { step(); }); // Run ASAP on the GUI thread
	}

	++m_iNbFrames;
	auto now = currentTime();
	int elapsedDur = toSeconds(now - m_fpsTime);
	if(m_animPlaying && elapsedDur > 1.0)
	{
		m_currentFPS = m_iNbFrames / elapsedDur;
		m_fpsTime = now;
		m_iNbFrames = 0;
	}
}

void PandaDocument::rewind()
{
#ifdef PANDA_LOG_EVENTS
	panda::helper::UpdateLogger::getInstance()->startLog(this);
#endif
	m_renderFBO.reset();
	m_animTimeVal = 0.0;
	m_animTime.setValue(0.0);
	m_mousePositionVal = m_mousePositionBuffer;
	m_mousePosition.setValue(m_mousePositionBuffer);
	m_mouseClickVal = 0;
	m_mouseClick.setValue(0);
	for(auto object : m_objects)
		object->reset();
	setDirtyValue(this);
	m_timeChangedSignal.run();
}

void PandaDocument::copyDataToUserValue(const BaseData* data)
{
	if(!data)
		return;

	auto entry = DataFactory::getInstance()->getEntry(data->getDataTrait()->fullTypeId());
	if(!entry)
		return;

	std::string registryName = std::string("panda::GeneratorUser<") + entry->className + ">";
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

void PandaDocument::addCommand(UndoCommand::SPtr command)
{
	if(m_resetting)
		return;

	auto oldCommand = m_currentCommand;
	m_currentCommand = command;
	m_undoStack.push(command);
	m_currentCommand = oldCommand;
}

std::shared_ptr<ScopedMacro> PandaDocument::beginCommandMacro(const std::string& text)
{
	m_undoStack.beginMacro(text);
	++m_inCommandMacro;
	return std::make_shared<ScopedMacro>(this);
}

void PandaDocument::endCommandMacro()
{
	--m_inCommandMacro;
	m_undoStack.endMacro();
}

void PandaDocument::clearCommands()
{
	m_undoStack.clear();
}

} // namespace panda


