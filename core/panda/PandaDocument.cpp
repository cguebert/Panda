#include <GL/glew.h>

#include <panda/PandaDocument.h>
#include <panda/Messaging.h>
#include <panda/SimpleGUI.h>
#include <panda/TimedFunctions.h>
#include <panda/UndoStack.h>
#include <panda/data/DataFactory.h>
#include <panda/document/DocumentRenderer.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/GraphUtils.h>
#include <panda/document/NodeUpdater.h>
#include <panda/document/Scheduler.h>
#include <panda/object/Layer.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/graphics/Framebuffer.h>
#include <panda/graphics/Model.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/GradientCache.h>
#include <panda/helper/ShaderCache.h>
#include <panda/helper/UpdateLogger.h>
#include <panda/helper/system/FileRepository.h>

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

PandaDocument::PandaDocument(gui::BaseGUI& gui)
	: PandaObject(nullptr)
	, m_currentIndex(1)
	, m_renderSize(initData(Point(800,600), "render size", "Size of the image to be rendered"))
	, m_backgroundColor(initData(Color::white(), "background color", "Background color of the image to be rendered"))
	, m_animTime(initData(0.0, "time", "Time of the animation"))
	, m_timestep(initData(0.01, "timestep", "Time step of the animation"))
	, m_useTimer(initData(1, "use timer", "If true, wait before the next timestep. If false, compute the next one as soon as the previous finished."))
	, m_mousePosition(initData("mouse position", "Current position of the mouse in the render view"))
	, m_mouseClick(initData(0, "mouse click", "1 if the left mouse button is pressed"))
	, m_nbThreads(initData(0, "nb threads", "Optimize computation for multiple CPU cores (not using the scheduler if < 0)"))
	, m_gui(gui)
	, m_renderer(std::make_unique<DocumentRenderer>(*this))
	, m_signals(std::make_unique<DocumentSignals>())
	, m_undoStack(std::make_unique<UndoStack>())
	, m_nodeUpdater(std::make_unique<NodeUpdater>(*this))
{
	addInput(m_renderSize);
	addInput(m_backgroundColor);
	addInput(m_timestep);
	addInput(m_useTimer);
	addInput(m_nbThreads);

	m_useTimer.setWidget("checkbox");

	// Not connecting to the document, otherwise it would update the layers each time we get the time.
	m_animTime.setOutput(true);
	m_animTime.setReadOnly(true);

	m_mousePosition.setOutput(true);
	m_mousePosition.setReadOnly(true);

	m_mouseClick.setOutput(true);
	m_mouseClick.setReadOnly(true);
	m_mouseClick.setWidget("checkbox");

	m_defaultLayer = std::make_shared<Layer>(this);
	m_defaultLayer->getLayerNameData().setValue("Default Layer");

	setInternalData("Document", 0);
	
	m_undoStack->setUndoLimit(25);

	setParentDocument(this);
}

PandaDocument::~PandaDocument()
{
	if(m_scheduler)
		m_scheduler->stop();

	m_resetting = true;
	m_undoStack->setEnabled(false);

	// Just to be sure everything goes smoothly
	for(auto object : m_objects)
		object->preDestruction();

	TimedFunctions::instance().shutdown();

	m_objects.clear();
	m_undoStack->clear();
}

void PandaDocument::resetDocument()
{
	m_resetting = true;
	m_undoStack->setEnabled(false);

	m_signals->clearDocument.run();

	for(auto object : m_objects)
	{
		m_signals->removedObject.run(object.get());
		object->preDestruction();
	}

	m_objects.clear();
	m_currentIndex = 1;
	m_animTimeVal = 0.0;
	m_animTime.setValue(0.0);
	m_timestep.setValue((float)0.01);
	m_useTimer.setValue(1);
	m_backgroundColor.setValue(Color::white());
	m_nbThreads.setValue(0);

	setRenderSize({ 800, 600 });

	m_animPlaying = false;
	m_animMultithread = false;
	TimedFunctions::instance().cancelAll();
	if(m_scheduler)
		m_scheduler->stop();

	m_undoStack->clear();

	helper::GradientCache::getInstance()->clear();
	helper::ShaderCache::getInstance()->clear();

	m_signals->modified.run();
	m_signals->timeChanged.run();

	m_resetting = false;
	m_undoStack->setEnabled(true);
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
	m_signals->reorderedObjects.run();
	m_signals->modified.run();
}

graphics::Size PandaDocument::getRenderSize() const
{
	Point pt = m_renderSize.getValue();
	return graphics::Size(std::max(1, static_cast<int>(pt.x)), std::max(1, static_cast<int>(pt.y)));
}

void PandaDocument::setRenderSize(graphics::Size size)
{ 
	m_renderSize.setValue(
		panda::types::Point(
			std::max(1.f, static_cast<float>(size.width())), 
			std::max(1.f, static_cast<float>(size.height()))
			)
		); 
}

void PandaDocument::mouseMoveEvent(types::Point localPos, types::Point globalPos)
{ 
	m_mousePositionBuffer = localPos;
	m_signals->mouseMoveEvent.run(localPos, globalPos);
}

void PandaDocument::mouseButtonEvent(int button, bool isPressed, types::Point pos)
{
	if (button == 0)
	{
		if (m_mouseClickBuffer && !isPressed) // Pressed & released in 1 timestep, we will send 2 events
			m_mouseClickBuffer = -1;
		else
			m_mouseClickBuffer = isPressed;
	}

	m_signals->mouseButtonEvent.run(button, isPressed, pos);
}

void PandaDocument::keyEvent(int key, bool isPressed)
{
	m_signals->keyEvent.run(key, isPressed);
}

void PandaDocument::textEvent(const std::string& text)
{
	m_signals->textEvent.run(text);
}

void PandaDocument::addObject(ObjectPtr object)
{
	m_objects.push_back(object);
	object->addedToDocument();
	m_signals->addedObject.run(object.get());
	m_signals->modified.run();
}

void PandaDocument::removeObject(PandaObject* object)
{
	object->removedFromDocument();
	m_signals->removedObject.run(object);
	helper::removeIf(m_objects, [object](const ObjectPtr& ptr){
		return ptr.get() == object;
	});

	m_signals->modified.run();
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

PandaObject* PandaDocument::findObject(uint32_t objectIndex)
{
	auto iter = std::find_if(m_objects.cbegin(), m_objects.cend(), [objectIndex](const ObjectPtr& object){
		return object->getIndex() == objectIndex;
	});

	if(iter != m_objects.end())
		return iter->get();

	return nullptr;
}

BaseData* PandaDocument::findData(uint32_t objectIndex, const std::string& dataName)
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

	if (isInStep())
	{
		m_dirtyObjects.push_back(object);
	}
	else
	{
		m_signals->dirtyObject.run(object);
		m_signals->modified.run();
	}
}

void PandaDocument::onModifiedObject(PandaObject* object)
{
	if(m_resetting)
		return;

	m_signals->modifiedObject.run(object);
	m_signals->modified.run();
}

void PandaDocument::onChangedDock(DockableObject* dockable)
{
	if(m_resetting)
		return;

	m_signals->changedDock.run(dockable);
}

void PandaDocument::update()
{
	helper::GradientCache::getInstance()->resetUsedFlag();
	helper::ShaderCache::getInstance()->resetUsedFlag();

	if (!m_renderer->renderingMainView()) // If it not already the case, make the OpenGL context current
	{
		helper::ScopedEvent log("context make current");
		m_gui.contextMakeCurrent();
	}

	if(m_animMultithread && m_scheduler)
		m_scheduler->update();

	m_renderer->renderGL();

	helper::GradientCache::getInstance()->clearUnused();
	helper::ShaderCache::getInstance()->clearUnused();

	if (!m_renderer->renderingMainView()) // Release the context if we made it current ourselves
	{
		helper::ScopedEvent log("context done current");
		m_gui.contextDoneCurrent();
	}
}

graphics::Framebuffer& PandaDocument::getFBO()
{
	updateIfDirty();
	return m_renderer->getFBO();
}

void PandaDocument::play(bool playing)
{
	m_animPlaying = playing;
	if(m_animPlaying)
	{
		int nbThreads = m_nbThreads.getValue();
		m_animMultithread = nbThreads != 0;
		if(m_animMultithread)
		{
			if(!m_scheduler)
				m_scheduler = std::make_unique<Scheduler>(this);
			m_scheduler->init(nbThreads);
		}
		else
		{
			helper::UpdateLogger::getInstance()->setNbThreads(1);
			helper::UpdateLogger::getInstance()->setupThread(0);
		}

		m_gui.executeByUI([this]() { step(); });

		m_iNbFrames = 0;
		m_fpsTime = currentTime();
	}
	else
	{
		if(m_animMultithread && m_scheduler)
			m_scheduler->stop();
		m_animMultithread = false;

		if (m_stepQueued && !TimedFunctions::instance().cancelRun(m_animFunctionIndex))
			m_stepCanceled = true; // Ignore the next step
	}
}

void PandaDocument::step()
{
	m_stepQueued = false;
	if (m_stepCanceled)	// The user clicked stop after a step has been queued
	{
		m_stepCanceled = false;
		return;
	}

	auto timeStepval = m_timestep.getValue();
	auto startTime = std::chrono::high_resolution_clock::now();
	panda::helper::UpdateLogger::getInstance()->startLog(this);

	setInStep(true);
	// Force the value of isInStep, because some objects will propagate dirtyValue during beginStep
	for(auto& object : m_objects)
		object->setInStep(true);

	// First update the value of the document (without modifying the corresponding Data)
	// This is so an object reacting on the time having changed can get the correct value of the mouse position (and not the one from the previous step)
	m_animTimeVal += timeStepval;
	m_mousePositionVal = m_mousePositionBuffer;
	if(m_mouseClickBuffer < 0)
	{
		m_mouseClickVal = 1;
		m_mouseClickBuffer = 0;
	}
	else
		m_mouseClickVal = m_mouseClickBuffer;

	if (m_animPlaying && m_animMultithread && m_scheduler)
		m_scheduler->setDirty();

	// Let some objects set dirtyValue for each step
	for(auto& object : m_objects)
		object->beginStep();

	// Update the documents Data (all the values are already correct if using the getters)
	m_animTime.setValue(m_animTimeVal);
	m_mousePosition.setValue(m_mousePositionVal);
	m_mouseClick.setValue(m_mouseClickVal);

	setDirtyValue(this);
	updateIfDirty();

	setInStep(false);
	for(auto& object : m_objects)
		object->endStep();

	panda::helper::UpdateLogger::getInstance()->stopLog();

	m_signals->timeChanged.run();

	for (auto obj : m_dirtyObjects)
		m_signals->dirtyObject.run(obj);
	m_dirtyObjects.clear();

	m_signals->modified.run();

	if (m_animPlaying)	
	{
		m_stepQueued = true; // We want another step
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
	float elapsedDur = static_cast<float>(toSeconds(now - m_fpsTime));
	if(m_animPlaying && elapsedDur > 1.0)
	{
		m_currentFPS = m_iNbFrames / elapsedDur;
		m_fpsTime = now;
		m_iNbFrames = 0;
	}
}

void PandaDocument::rewind()
{
	panda::helper::UpdateLogger::getInstance()->startLog(this);
	m_animTimeVal = 0.0;
	m_animTime.setValue(0.0);
	m_mousePositionVal = m_mousePositionBuffer;
	m_mousePosition.setValue(m_mousePositionBuffer);
	m_mouseClickVal = 0;
	m_mouseClick.setValue(0);
	for(auto object : m_objects)
		object->reset();
	setDirtyValue(this);
	m_signals->timeChanged.run();
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

	BaseData* captionData = object->getData("caption");
	if (captionData)
	{
		auto captionTextData = dynamic_cast<Data<std::string>*>(captionData);
		if (captionTextData)
			captionTextData->setValue(data->getName());
	}
}

} // namespace panda


