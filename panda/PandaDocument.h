#ifndef PANDADOCUMENT_H
#define PANDADOCUMENT_H

#include <panda/PandaObject.h>

#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/types/ImageWrapper.h>

#include <QObject>
#include <QSize>
#include <QMap>
#include <QDomDocument>

class QOpenGLFramebufferObject;

namespace panda {

class BaseLayer;
class Layer;
class Group;
class Scheduler;

class PandaDocument : public PandaObject
{
	Q_OBJECT
public:
	PANDA_CLASS(PandaDocument, PandaObject)

	typedef QList<PandaObject*> ObjectsList;
	typedef QMapIterator<QString, QString> GroupsIterator;

	explicit PandaDocument(QObject *parent = 0);
	~PandaDocument();

	bool writeFile(const QString& fileName);
	bool readFile(const QString& fileName, bool isImport=false);

	QString writeTextDocument();
	bool readTextDocument(QString &text);

	bool saveDoc(QDomDocument& doc, QDomElement& root, const QList<PandaObject*>& selected);
	bool loadDoc(QDomElement& root);

	void resetDocument();

	PandaObject* createObject(QString registryName);
	int getNbObjects() const;
	const ObjectsList getObjects() const;

	PandaObject* getCurrentSelectedObject();
	void setCurrentSelectedObject(PandaObject* object);
	bool isSelected(PandaObject* object) const;
	int getNbSelected() const;
	const ObjectsList getSelection() const;

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

	void doAddObject(PandaObject* object);
	void doRemoveObject(PandaObject* object, bool del=true);

	void createGroupsList();
	GroupsIterator getGroupsIterator();
	QString getGroupDescription(const QString& groupName);
	bool saveGroup(Group* group);
	PandaObject* createGroupObject(QString groupPath);

protected:
	ObjectsList pandaObjects, selectedObjects;
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

	QMap<QString, QString> groupsMap;

	QSharedPointer<Scheduler> m_scheduler;

	bool getGroupDescription(const QString &fileName, QString& description);
	void render();

private:
	QString groupsDirPath;

signals:
	void modified();
	void modifiedObject(panda::PandaObject*);
	void addedObject(panda::PandaObject*);
	void removedObject(panda::PandaObject*);
	void savingObject(QDomDocument&, QDomElement&, panda::PandaObject*);
	void loadingObject(QDomElement&, panda::PandaObject*);
	void selectedObject(panda::PandaObject*);
	void selectedObjectIsDirty(panda::PandaObject*);
	void selectionChanged();
	void timeChanged();

public slots:
	void cut();
	void copy();
	void paste();
	void del();
	void selectionAdd(panda::PandaObject* object);
	void selectionRemove(panda::PandaObject* object);
	void selectAll();
	void selectNone();
	void selectConnected();
	void onDirtyObject(panda::PandaObject* object);
	void play(bool playing);
	void step();
	void rewind();
	void copyDataToUserValue(const panda::BaseData* data);
};

inline int PandaDocument::getNbObjects() const
{ return pandaObjects.size(); }

inline const PandaDocument::ObjectsList PandaDocument::getObjects() const
{ return pandaObjects; }

inline bool PandaDocument::isSelected(PandaObject* object) const
{ return selectedObjects.contains(object); }

inline int PandaDocument::getNbSelected() const
{ return selectedObjects.size(); }

inline const PandaDocument::ObjectsList PandaDocument::getSelection() const
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

inline PandaDocument::GroupsIterator PandaDocument::getGroupsIterator()
{ return GroupsIterator(groupsMap); }

inline QString PandaDocument::getGroupDescription(const QString& groupName)
{ return groupsMap.value(groupName); }

inline quint32 PandaDocument::getNextIndex()
{ return currentIndex++; }

inline Layer* PandaDocument::getDefaultLayer()
{ return defaultLayer; }

} // namespace panda

#endif // PANDADOCUMENT_H
