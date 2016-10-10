#include <panda/document/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/TimedFunctions.h>
#include <panda/UndoStack.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/ObjectsList.h>
#include <panda/document/Scheduler.h>
#include <panda/helper/UpdateLogger.h>

#include <chrono>

namespace
{

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

PandaDocument::PandaDocument(gui::BaseGUI& gui)
	: PandaObject(nullptr)
	, m_currentIndex(1)
	, m_animTime(initData(0.0, "time", "Time of the animation"))
	, m_timestep(initData(0.01, "timestep", "Time step of the animation"))
	, m_useTimer(initData(1, "use timer", "If true, wait before the next timestep. If false, compute the next one as soon as the previous finished."))
	, m_nbThreads(initData(0, "nb threads", "Optimize computation for multiple CPU cores (not using the scheduler if < 0)"))
	, m_gui(gui)
	, m_objectsList(std::make_unique<ObjectsList>())
	, m_signals(std::make_unique<DocumentSignals>())
	, m_undoStack(std::make_unique<UndoStack>())
{
	addInput(m_timestep);
	addInput(m_useTimer);
	addInput(m_nbThreads);

	m_useTimer.setWidget("checkbox");

	// Not connecting to the document, otherwise it would update the layers each time we get the time.
	m_animTime.setOutput(true);
	m_animTime.setReadOnly(true);

	setInternalData("Document", 0);
	
	m_undoStack->setUndoLimit(25);

	setParentDocument(this);
}

PandaDocument::~PandaDocument()
{
	if(m_scheduler)
		m_scheduler->stop();

	m_isResetting = true;
	m_undoStack->setEnabled(false);

	// Just to be sure everything goes smoothly
	m_objectsList->clear(false);
	m_undoStack->clear();
}

void PandaDocument::resetDocument()
{
	m_isResetting = true;
	m_undoStack->setEnabled(false);

	m_objectsList->clear();
	m_currentIndex = 1;
	m_animTimeVal = 0.0;
	m_animTime.setValue(0.0);
	m_timestep.setValue((float)0.01);
	m_useTimer.setValue(1);
	m_nbThreads.setValue(0);

	m_animPlaying = false;
	m_animMultithread = false;
	TimedFunctions::instance().cancelAll();
	if(m_scheduler)
		m_scheduler->stop();

	m_undoStack->clear();

	m_signals->modified.run();
	m_signals->timeChanged.run();

	m_isResetting = false;
	m_undoStack->setEnabled(true);
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

void PandaDocument::onDirtyObject(PandaObject* object)
{
	if(m_isResetting)
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
	if(m_isResetting)
		return;

	m_signals->modifiedObject.run(object);
	m_signals->modified.run();
}

void PandaDocument::onChangedDock(DockableObject* dockable)
{
	if(m_isResetting)
		return;

	m_signals->changedDock.run(dockable);
}

void PandaDocument::update()
{
	if(m_animMultithread && m_scheduler)
		m_scheduler->update();
}

void PandaDocument::play(bool playing)
{
	m_animPlaying = playing;
	if(m_animPlaying)
	{
		m_timeStepVal = m_timestep.getValue();
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

	auto startTime = std::chrono::high_resolution_clock::now();
	panda::helper::UpdateLogger::getInstance()->startLog(this);

	setInStep(true);
	// Force the value of isInStep, because some objects will propagate dirtyValue during beginStep
	for(auto& object : m_objectsList->get())
		object->setInStep(true);

	updateDocumentData();

	setDirtyValue(this);
	updateIfDirty();

	setInStep(false);
	for(auto& object : m_objectsList->get())
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
	m_animTimeVal = 0.0;
	m_animTime.setValue(0.0);
	for(auto object : m_objectsList->get())
		object->reset();
	setDirtyValue(this);
	m_signals->timeChanged.run();
}

void PandaDocument::updateDocumentData()
{
	// First update the value of the document (without modifying the corresponding Data)
	// This is so an object reacting on the time having changed can get the correct value of the mouse position (and not the one from the previous step)
	m_animTimeVal += m_timeStepVal;

	if (m_animPlaying && m_animMultithread && m_scheduler)
		m_scheduler->setDirty();

	// Let some objects set dirtyValue for each step
	for(auto& object : m_objectsList->get())
		object->beginStep();

	// Update the documents Data (all the values are already correct if using the getters)
	m_animTime.setValue(m_animTimeVal);
}

} // namespace panda


