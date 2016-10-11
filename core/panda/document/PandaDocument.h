#ifndef PANDADOCUMENT_H
#define PANDADOCUMENT_H

#include <panda/object/PandaObject.h>

namespace panda {

class DockableObject;
class DocumentSignals;
class ObjectsList;
class Scheduler;
class UndoStack;

namespace gui
{
	class BaseGUI;
}

class PANDA_CORE_API PandaDocument : public PandaObject
{
public:
	PANDA_CLASS(PandaDocument, PandaObject)

	explicit PandaDocument(gui::BaseGUI& gui);
	~PandaDocument();

	bool isResetting() const;

	float getAnimationTime() const;
	float getTimeStep() const;
	float getFPS() const;
	bool animationIsPlaying() const;
	bool animationIsMultithread() const;

	uint32_t getNextIndex();

	void update() override;

	// When an object is set to laterUpdate, use these functions to help the Scheduler
	void setDataDirty(BaseData* data) const; // Set the outputs to dirty before setting the value (so it doesn't propagate)
	void setDataReady(BaseData* data) const; // Launch the tasks connected to this node
	void waitForOtherTasksToFinish(bool mainThread = true) const; // Wait until the tasks we launched finish

	void onDirtyObject(PandaObject* object);
	void onModifiedObject(PandaObject* object);
	void onChangedDock(DockableObject* dockable); // When the dockable has changed dock

	gui::BaseGUI& getGUI() const; // Access to the GUI thread, update the view, show message boxes
	ObjectsList& getObjectsList() const; // Access to the objects, signals when modified
	DocumentSignals& getSignals() const; // Connect and run signals for when the document is modified
	UndoStack& getUndoStack() const; // Undo/redo capabilities

	// Slots or called only by the UI
	void play(bool playing);
	void step();
	virtual void rewind();

protected:
	virtual void updateDocumentData(); // At the start of a step

	using ObjectsRawList = std::vector<PandaObject*>;
	ObjectsRawList m_dirtyObjects; // All the objects that were dirty during the current step
	uint32_t m_currentIndex;

	float m_animTimeVal = 0, m_timeStepVal = 0;

	Data<float> m_animTime, m_timestep;
	Data<int> m_useTimer;
	Data<int> m_nbThreads;

	bool m_isResetting = false;

	bool m_animPlaying = false, m_animMultithread = false;
	bool m_stepQueued = false, m_stepCanceled = false;
	int m_animFunctionIndex = -1;

	int m_iNbFrames = 0;
	long long m_fpsTime = 0;
	float m_currentFPS = 0;

	gui::BaseGUI& m_gui;
	std::unique_ptr<ObjectsList> m_objectsList;
	std::unique_ptr<DocumentSignals> m_signals;
	std::unique_ptr<Scheduler> m_scheduler;
	std::unique_ptr<UndoStack> m_undoStack;
};

//****************************************************************************//

inline bool PandaDocument::isResetting() const
{ return m_isResetting; }

inline float PandaDocument::getAnimationTime() const
{ return m_animTimeVal; }

inline float PandaDocument::getTimeStep() const
{ return m_timestep.getValue(); }

inline float PandaDocument::getFPS() const
{ return m_currentFPS; }

inline bool PandaDocument::animationIsPlaying() const
{ return m_animPlaying; }

inline bool PandaDocument::animationIsMultithread() const
{ return m_animMultithread; }

inline uint32_t PandaDocument::getNextIndex()
{ return m_currentIndex++; }

inline gui::BaseGUI& PandaDocument::getGUI() const
{ return m_gui; }

inline ObjectsList& PandaDocument::getObjectsList() const
{ return *m_objectsList; }

inline DocumentSignals& PandaDocument::getSignals() const
{ return *m_signals; }

inline UndoStack& PandaDocument::getUndoStack() const
{ return *m_undoStack; }

} // namespace panda

#endif // PANDADOCUMENT_H
