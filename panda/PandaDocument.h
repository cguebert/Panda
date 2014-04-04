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

	types::Point mousePositionBuffer;
	int mouseClickBuffer;

	bool animPlaying;
	QTimer* animTimer;

	QMap<QString, QString> groupsMap;

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

} // namespace panda

#endif // PANDADOCUMENT_H
