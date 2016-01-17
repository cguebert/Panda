#ifndef PANDADOCUMENT_H
#define PANDADOCUMENT_H

#include <panda/core.h>
#include <panda/object/PandaObject.h>

#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>

#include <panda/graphics/Size.h>

#include <panda/UndoStack.h>

namespace panda {

class BaseLayer;
class Layer;
class Scheduler;
class ScopedMacro;
class DockableObject;
class XmlElement;

namespace graphics
{
	class Framebuffer;
	class ShaderProgram; 
}

namespace gui
{
	class BaseGUI;
}

class PANDA_CORE_API PandaDocument : public PandaObject
{
public:
	PANDA_CLASS(PandaDocument, PandaObject)

	typedef std::shared_ptr<PandaObject> ObjectPtr;
	typedef std::vector<ObjectPtr> ObjectsList;
	typedef std::vector<PandaObject*> ObjectsSelection;

	explicit PandaDocument(gui::BaseGUI& gui);
	~PandaDocument();

	bool writeFile(const std::string& fileName);
	bool readFile(const std::string& fileName, bool isImport=false);

	std::string writeTextDocument();
	bool readTextDocument(const std::string& text);

	bool saveDoc(XmlElement& root, const ObjectsSelection& selected);
	bool loadDoc(XmlElement& root);

	void resetDocument();

	void addObject(ObjectPtr object);
	void removeObject(PandaObject* object);

	int getNbObjects() const;
	const ObjectsList& getObjects() const;
	ObjectPtr getSharedPointer(PandaObject* object) const;

	int getObjectPosition(PandaObject* object) const; /// Get the object's position in the objects list
	void reinsertObject(PandaObject* object, int pos); /// Reorder the object so it is inserted at the index pos in the objects list

	PandaObject* getCurrentSelectedObject() const;
	void setCurrentSelectedObject(PandaObject* object);
	bool isSelected(PandaObject* object) const;
	int getNbSelected() const;
	const ObjectsSelection& getSelection() const;

	types::Color getBackgroundColor() const;
	void setBackgroundColor(types::Color color);

	graphics::Size getRenderSize() const;
	void setRenderSize(graphics::Size size);

	PReal getAnimationTime() const;
	PReal getTimeStep() const;
	PReal getFPS() const;
	bool animationIsPlaying() const;
	bool animationIsMultithread() const;

	types::Point getMousePosition() const;
	void setMousePosition(const types::Point& pos);

	int getMouseClick() const;
	void setMouseClick(bool clicked, const types::Point& pos);

	uint32_t getNextIndex();
	PandaObject* findObject(uint32_t objectIndex);
	BaseData* findData(uint32_t objectIndex, const std::string& dataName);

	virtual void update();
	virtual void setDirtyValue(const DataNode* caller);

	const types::ImageWrapper& getRenderedImage();
	std::shared_ptr<graphics::Framebuffer> getFBO();

	Layer* getDefaultLayer() const;

	// When an object is set to laterUpdate, use these functions to help the Scheduler
	void setDataDirty(BaseData* data) const; // Set the outputs to dirty before setting the value (so it doesn't propagate)
	void setDataReady(BaseData* data) const; // Launch the tasks connected to this node
	void waitForOtherTasksToFinish(bool mainThread = true) const; // Wait until the tasks we launched finish

	// For undo-redo actions
	void addCommand(UndoCommand::SPtr command);
	std::shared_ptr<ScopedMacro> beginCommandMacro(const std::string& text);
	void clearCommands();
	bool isInCommandMacro() const;
	UndoCommand* getCurrentCommand() const; /// The command we are currently adding (if we want to connect another to this one)
	UndoStack& undoStack();

	void onDirtyObject(PandaObject* object);
	void onModifiedObject(PandaObject* object);
	void onChangedDock(DockableObject* dockable); // When the dockable has changed dock

	void initializeGL(); // Called by the render view, (PandaCore does not link with OpenGL)
	bool isGLInitialized() const; // Can the objects use OpenGL yet?

	gui::BaseGUI& getGUI() const;

protected:
	friend class ScopedMacro;

	void render();
	void endCommandMacro();

	ObjectsList m_objects;
	ObjectsSelection m_selectedObjects;
	ObjectsSelection m_dirtyObjects; // All the objects that were dirty during the current step
	uint32_t m_currentIndex;
	std::shared_ptr<Layer> m_defaultLayer;
	std::shared_ptr<graphics::Framebuffer> m_renderFBO, m_secondRenderFBO;
	std::shared_ptr<graphics::ShaderProgram> m_mergeLayersShader;

	PReal m_animTimeVal = 0.0;
	types::Point m_mousePositionVal;
	int m_mouseClickVal = 0;

	Data<types::Point> m_renderSize;
	Data<types::Color> m_backgroundColor;
	Data<PReal> m_animTime, m_timestep;
	Data<int> m_useTimer;
	Data<types::Point> m_mousePosition;
	Data<int> m_mouseClick;
	Data<types::ImageWrapper> m_renderedImage;
	Data<int> m_useMultithread;

	types::Point m_mousePositionBuffer;
	int m_mouseClickBuffer = 0;

	int m_inCommandMacro = 0;
	bool m_resetting = false;

	bool m_isGLInitialized = false;

	bool m_animPlaying = false, m_animMultithread = false;
	bool m_stepQueued = false, m_stepCanceled = false;
	int m_animFunctionIndex = -1;

	std::shared_ptr<Scheduler> m_scheduler;

	UndoStack m_undoStack;
	UndoCommand::SPtr m_currentCommand;

	int m_iNbFrames = 0;
	long long m_fpsTime = 0;
	PReal m_currentFPS = 0;

	gui::BaseGUI& m_gui;

public:
// Signals
	msg::Signal<void()> m_modifiedSignal;
	msg::Signal<void(panda::PandaObject*)> m_modifiedObjectSignal;
	msg::Signal<void(panda::PandaObject*)> m_dirtyObjectSignal;
	msg::Signal<void(panda::PandaObject*)> m_addedObjectSignal;
	msg::Signal<void(panda::PandaObject*)> m_removedObjectSignal;
	msg::Signal<void(panda::PandaObject*)> m_selectedObjectSignal;
	msg::Signal<void(panda::PandaObject*)> m_selectedObjectIsDirtySignal;
	msg::Signal<void(XmlElement&, panda::PandaObject*)> m_savingObjectSignal;
	msg::Signal<void(XmlElement&, panda::PandaObject*)> m_loadingObjectSignal;
	msg::Signal<void()> m_selectionChangedSignal;
	msg::Signal<void()> m_timeChangedSignal;
	msg::Signal<void()> m_renderSizeChangedSignal;
	msg::Signal<void()> m_reorderedObjectsSignal;
	msg::Signal<void()> m_startLoadingSignal;
	msg::Signal<void()> m_loadingFinishedSignal;
	msg::Signal<void(panda::DockableObject*)> m_changedDockSignal;
	msg::Signal<void(panda::types::Point)> m_mousePressedSignal;
	msg::Signal<void(panda::types::Point)> m_mouseReleasedSignal;

// Slots or called only by the UI
	void selectionAdd(panda::PandaObject* object);
	void selectionRemove(panda::PandaObject* object);
	void selectAll();
	void selectNone();
	void selectConnected();
	void play(bool playing);
	void step();
	void rewind();
	void copyDataToUserValue(const panda::BaseData* data);
};

class ScopedMacro
{
public:
	ScopedMacro(PandaDocument* doc) : m_document(doc) {}
	~ScopedMacro() { m_document->endCommandMacro(); }

protected:
	PandaDocument* m_document;
};

//****************************************************************************//

inline int PandaDocument::getNbObjects() const
{ return m_objects.size(); }

inline const PandaDocument::ObjectsList& PandaDocument::getObjects() const
{ return m_objects; }

inline bool PandaDocument::isSelected(PandaObject* object) const
{ return std::find(m_selectedObjects.begin(), m_selectedObjects.end(), object) != m_selectedObjects.end(); }

inline int PandaDocument::getNbSelected() const
{ return m_selectedObjects.size(); }

inline const PandaDocument::ObjectsSelection& PandaDocument::getSelection() const
{ return m_selectedObjects; }

inline types::Color PandaDocument::getBackgroundColor() const
{ return m_backgroundColor.getValue(); }

inline void PandaDocument::setBackgroundColor(types::Color color)
{ m_backgroundColor.setValue(color); }

inline PReal PandaDocument::getAnimationTime() const
{ return m_animTimeVal; }

inline PReal PandaDocument::getTimeStep() const
{ return m_timestep.getValue(); }

inline PReal PandaDocument::getFPS() const
{ return m_currentFPS; }

inline bool PandaDocument::animationIsPlaying() const
{ return m_animPlaying; }

inline bool PandaDocument::animationIsMultithread() const
{ return m_animMultithread; }

inline types::Point PandaDocument::getMousePosition() const
{ return m_mousePositionVal; }

inline void PandaDocument::setMousePosition(const types::Point& pos)
{ m_mousePositionBuffer = pos; }

inline int PandaDocument::getMouseClick() const
{ return m_mouseClickVal; }

inline uint32_t PandaDocument::getNextIndex()
{ return m_currentIndex++; }

inline Layer* PandaDocument::getDefaultLayer() const
{ return m_defaultLayer.get(); }

inline bool PandaDocument::isInCommandMacro() const
{ return m_inCommandMacro > 0; }

inline UndoCommand* PandaDocument::getCurrentCommand() const
{ return m_currentCommand.get(); }

inline UndoStack& PandaDocument::undoStack()
{ return m_undoStack; }

inline void PandaDocument::initializeGL()
{ m_isGLInitialized = true; }

inline bool PandaDocument::isGLInitialized() const
{ return m_isGLInitialized; }

inline gui::BaseGUI& PandaDocument::getGUI() const
{ return m_gui; }

} // namespace panda

#endif // PANDADOCUMENT_H
