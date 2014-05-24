#ifndef PANDADOCUMENT_H
#define PANDADOCUMENT_H

#include <panda/PandaObject.h>

#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>

#include <QObject>
#include <QSize>
#include <QMap>

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

	PandaObject* createObject(QString registryName);

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
	ObjectsList pandaObjects;
	ObjectsSelection selectedObjects;
	QMap<quint32, PandaObject*> pandaObjectsMap;
	quint32 currentIndex;
	Layer* defaultLayer;
	QSharedPointer<QOpenGLFramebufferObject> renderFrameBuffer;

	Data<types::Point> renderSize;
	Data<types::Color> backgroundColor;
	Data<PReal> animTime, timestep;
	Data<int> useTimer;
	Data<types::Point> mousePosition;
	Data<int> mouseClick;
	Data<types::ImageWrapper> renderedImage;
	Data<int> useMultithread;

	types::Point mousePositionBuffer;
	int mouseClickBuffer;

	bool animPlaying, animMultithread;
	QTimer* animTimer;

	QSharedPointer<Scheduler> m_scheduler;

	QUndoStack* m_undoStack;
	QUndoCommand* m_currentCommand;

	void render();

	int m_inCommandMacro;
	friend class ScopedMacro;
	void endCommandMacro();

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
{ return pandaObjects.size(); }

inline const PandaDocument::ObjectsList& PandaDocument::getObjects() const
{ return pandaObjects; }

inline bool PandaDocument::isSelected(PandaObject* object) const
{ return std::find(selectedObjects.begin(), selectedObjects.end(), object) != selectedObjects.end(); }

inline int PandaDocument::getNbSelected() const
{ return selectedObjects.size(); }

inline const PandaDocument::ObjectsSelection& PandaDocument::getSelection() const
{ return selectedObjects; }

inline types::Color PandaDocument::getBackgroundColor()
{ return backgroundColor.getValue(); }

inline void PandaDocument::setBackgroundColor(types::Color color)
{ backgroundColor.setValue(color); }

inline PReal PandaDocument::getAnimationTime()
{ return animTime.getValue(); }

inline PReal PandaDocument::getTimeStep()
{ return timestep.getValue(); }

inline bool PandaDocument::animationIsPlaying() const
{ return animPlaying; }

inline bool PandaDocument::animationIsMultithread() const
{ return animMultithread; }

inline types::Point PandaDocument::getMousePosition()
{ return mousePosition.getValue(); }

inline void PandaDocument::setMousePosition(const types::Point& pos)
{ mousePositionBuffer = pos; }

inline int PandaDocument::getMouseClick()
{ return mouseClick.getValue(); }

inline quint32 PandaDocument::getNextIndex()
{ return currentIndex++; }

inline Layer* PandaDocument::getDefaultLayer()
{ return defaultLayer; }

inline bool PandaDocument::isInCommandMacro()
{ return m_inCommandMacro > 0; }

inline QUndoCommand* PandaDocument::getCurrentCommand()
{ return m_currentCommand; }

} // namespace panda

#endif // PANDADOCUMENT_H
