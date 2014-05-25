#ifndef PANDADOCUMENT_H
#define PANDADOCUMENT_H

#include <panda/PandaObject.h>

#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>

#include <QObject>
#include <QSize>

class QOpenGLFramebufferObject;
class QAction;
class QUndoCommand;
class QUndoStack;

namespace panda {

class BaseLayer;
class Layer;
class Scheduler;
class ScopedMacro;

class PandaDocument : public QObject, public PandaObject
{
	Q_OBJECT
public:
	PANDA_CLASS(PandaDocument, PandaObject)

	typedef QSharedPointer<PandaObject> ObjectPtr;
	typedef QVector<ObjectPtr> ObjectsList;
	typedef QList<PandaObject*> ObjectsSelection;

	explicit PandaDocument(QObject *parent = 0);
	~PandaDocument();

	bool writeFile(const QString& fileName);
	bool readFile(const QString& fileName, bool isImport=false);

	QString writeTextDocument();
	bool readTextDocument(QString &text);

	bool saveDoc(QDomDocument& doc, QDomElement& root, const ObjectsSelection& selected);
	bool loadDoc(QDomElement& root);

	void resetDocument();

	void addObject(ObjectPtr object);
	void removeObject(PandaObject* object);

	int getNbObjects() const;
	const ObjectsList& getObjects() const;
	ObjectPtr getSharedPointer(PandaObject* object);

	PandaObject* getCurrentSelectedObject();
	void setCurrentSelectedObject(PandaObject* object);
	bool isSelected(PandaObject* object) const;
	int getNbSelected() const;
	const ObjectsSelection& getSelection() const;

	types::Color getBackgroundColor();
	void setBackgroundColor(types::Color color);
	QSize getRenderSize();

	PReal getAnimationTime();
	PReal getTimeStep();
	bool animationIsPlaying() const;
	bool animationIsMultithread() const;

	types::Point getMousePosition();
	void setMousePosition(const types::Point& pos);
	int getMouseClick();
	void setMouseClick(int state);

	quint32 getNextIndex();
	PandaObject* findObject(quint32 objectIndex);
	BaseData* findData(quint32 objectIndex, const QString& dataName);

	virtual void update();
	virtual void setDirtyValue();

	const types::ImageWrapper& getRenderedImage();
	QSharedPointer<QOpenGLFramebufferObject> getFBO();

	Layer* getDefaultLayer();
	void moveLayerUp(PandaObject *layer);
	void moveLayerDown(PandaObject* layer);

	// When an object is set to laterUpdate, use these functions to help the Scheduler
	void setDataDirty(BaseData* data); // Set the outputs to dirty before setting the value (so it doesn't propagate)
	void setDataReady(BaseData* data); // Launch the tasks connected to this node
	void waitForOtherTasksToFinish(bool mainThread = true); // Wait until the tasks we launched finish

	// For undo-redo actions
	void createUndoRedoActions(QObject* parent, QAction*& undoAction, QAction*& redoAction);
	void addCommand(QUndoCommand* command);
	ScopedMacro beginCommandMacro(QString text);
	void clearCommands();
	bool isInCommandMacro();
	QUndoCommand* getCurrentCommand(); /// The command we are currently adding (if we want to connect another to this one)

protected:
	friend class ScopedMacro;

	void render();
	void endCommandMacro();

	ObjectsList m_objects;
	ObjectsSelection m_selectedObjects;
	quint32 m_currentIndex;
	Layer* m_defaultLayer;
	QSharedPointer<QOpenGLFramebufferObject> m_renderFrameBuffer;

	Data<types::Point> m_renderSize;
	Data<types::Color> m_backgroundColor;
	Data<PReal> m_animTime, m_timestep;
	Data<int> m_useTimer;
	Data<types::Point> m_mousePosition;
	Data<int> m_mouseClick;
	Data<types::ImageWrapper> m_renderedImage;
	Data<int> m_useMultithread;

	types::Point m_mousePositionBuffer;
	int m_mouseClickBuffer;

	bool m_animPlaying, m_animMultithread;
	QTimer* m_animTimer;

	QSharedPointer<Scheduler> m_scheduler;

	QUndoStack* m_undoStack;
	QUndoCommand* m_currentCommand;

	int m_inCommandMacro;

signals:
	void modified();
	void modifiedObject(panda::PandaObject*);
	void dirtyObject(panda::PandaObject*);
	void addedObject(panda::PandaObject*);
	void removedObject(panda::PandaObject*);
	void savingObject(QDomDocument&, QDomElement&, panda::PandaObject*);
	void loadingObject(QDomElement&, panda::PandaObject*);
	void selectedObject(panda::PandaObject*);
	void selectedObjectIsDirty(panda::PandaObject*);
	void selectionChanged();
	void timeChanged();

public slots:
	void copy();
	void paste();
	void selectionAdd(panda::PandaObject* object);
	void selectionRemove(panda::PandaObject* object);
	void selectAll();
	void selectNone();
	void selectConnected();
	void onDirtyObject(panda::PandaObject* object);
	void onModifiedObject(panda::PandaObject* object);
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

inline types::Color PandaDocument::getBackgroundColor()
{ return m_backgroundColor.getValue(); }

inline void PandaDocument::setBackgroundColor(types::Color color)
{ m_backgroundColor.setValue(color); }

inline PReal PandaDocument::getAnimationTime()
{ return m_animTime.getValue(); }

inline PReal PandaDocument::getTimeStep()
{ return m_timestep.getValue(); }

inline bool PandaDocument::animationIsPlaying() const
{ return m_animPlaying; }

inline bool PandaDocument::animationIsMultithread() const
{ return m_animMultithread; }

inline types::Point PandaDocument::getMousePosition()
{ return m_mousePosition.getValue(); }

inline void PandaDocument::setMousePosition(const types::Point& pos)
{ m_mousePositionBuffer = pos; }

inline int PandaDocument::getMouseClick()
{ return m_mouseClick.getValue(); }

inline quint32 PandaDocument::getNextIndex()
{ return m_currentIndex++; }

inline Layer* PandaDocument::getDefaultLayer()
{ return m_defaultLayer; }

inline bool PandaDocument::isInCommandMacro()
{ return m_inCommandMacro > 0; }

inline QUndoCommand* PandaDocument::getCurrentCommand()
{ return m_currentCommand; }

} // namespace panda

#endif // PANDADOCUMENT_H
