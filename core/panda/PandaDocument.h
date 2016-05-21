#ifndef PANDADOCUMENT_H
#define PANDADOCUMENT_H

#include <panda/core.h>
#include <panda/object/PandaObject.h>

#include <panda/types/Color.h>
#include <panda/types/Point.h>

#include <panda/graphics/Size.h>

namespace panda {

class BaseLayer;
class DockableObject;
class DocumentRenderer;
class DocumentSignals;
class Layer;
class ObjectsList;
class Scheduler;
class UndoStack;
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

	explicit PandaDocument(gui::BaseGUI& gui);
	~PandaDocument();

	void resetDocument();

	types::Color getBackgroundColor() const;
	void setBackgroundColor(types::Color color);

	graphics::Size getRenderSize() const;
	void setRenderSize(graphics::Size size);

	float getAnimationTime() const;
	float getTimeStep() const;
	float getFPS() const;
	bool animationIsPlaying() const;
	bool animationIsMultithread() const;

	types::Point getMousePosition() const;
	void mouseMoveEvent(types::Point localPos, types::Point globalPos);

	int getMouseClick() const; // Only gives the status of the left button
	void mouseButtonEvent(int button, bool isPressed, types::Point pos);

	void keyEvent(int key, bool isPressed);
	void textEvent(const std::string& text); // Unicode inputs

	uint32_t getNextIndex();
	BaseData* findData(uint32_t objectIndex, const std::string& dataName);

	void update() override;

	graphics::Framebuffer& getFBO();

	Layer* getDefaultLayer() const;

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
	DocumentRenderer& getRenderer() const; // What takes care of rendering the document in OpenGL
	UndoStack& getUndoStack() const; // Undo/redo capabilities

	// Slots or called only by the UI
	void play(bool playing);
	void step();
	void rewind();
	void copyDataToUserValue(const panda::BaseData* data, ObjectsList& objectList);

protected:
	void render();

	using ObjectsRawList = std::vector<PandaObject*>;
	ObjectsRawList m_dirtyObjects; // All the objects that were dirty during the current step
	uint32_t m_currentIndex;
	std::shared_ptr<Layer> m_defaultLayer;

	float m_animTimeVal = 0.0;
	types::Point m_mousePositionVal;
	int m_mouseClickVal = 0;

	Data<types::Point> m_renderSize;
	Data<types::Color> m_backgroundColor;
	Data<float> m_animTime, m_timestep;
	Data<int> m_useTimer;
	Data<types::Point> m_mousePosition;
	Data<int> m_mouseClick;
	Data<int> m_nbThreads;

	types::Point m_mousePositionBuffer;
	int m_mouseClickBuffer = 0;

	bool m_resetting = false;

	bool m_animPlaying = false, m_animMultithread = false;
	bool m_stepQueued = false, m_stepCanceled = false;
	int m_animFunctionIndex = -1;

	int m_iNbFrames = 0;
	long long m_fpsTime = 0;
	float m_currentFPS = 0;

	gui::BaseGUI& m_gui;
	std::unique_ptr<ObjectsList> m_objectsList;
	std::unique_ptr<DocumentRenderer> m_renderer;
	std::unique_ptr<DocumentSignals> m_signals;
	std::unique_ptr<Scheduler> m_scheduler;
	std::unique_ptr<UndoStack> m_undoStack;
};

//****************************************************************************//

inline types::Color PandaDocument::getBackgroundColor() const
{ return m_backgroundColor.getValue(); }

inline void PandaDocument::setBackgroundColor(types::Color color)
{ m_backgroundColor.setValue(color); }

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

inline types::Point PandaDocument::getMousePosition() const
{ return m_mousePositionVal; }

inline int PandaDocument::getMouseClick() const
{ return m_mouseClickVal; }

inline uint32_t PandaDocument::getNextIndex()
{ return m_currentIndex++; }

inline Layer* PandaDocument::getDefaultLayer() const
{ return m_defaultLayer.get(); }

inline gui::BaseGUI& PandaDocument::getGUI() const
{ return m_gui; }

inline ObjectsList& PandaDocument::getObjectsList() const
{ return *m_objectsList; }

inline DocumentRenderer& PandaDocument::getRenderer() const
{ return *m_renderer; }

inline DocumentSignals& PandaDocument::getSignals() const
{ return *m_signals; }

inline UndoStack& PandaDocument::getUndoStack() const
{ return *m_undoStack; }

} // namespace panda

#endif // PANDADOCUMENT_H
