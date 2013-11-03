#ifndef PANDADOCUMENT_H
#define PANDADOCUMENT_H

#include <panda/PandaObject.h>
#include <QObject>
#include <QColor>
#include <QPointF>
#include <QSize>
#include <QMap>
#include <QImage>
#include <QDomDocument>

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
	bool readFile(const QString& fileName);

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

	QColor getBackgroundColor();
	void setBackgroundColor(QColor color);
	QSize getRenderSize();

	double getAnimationTime();
	double getTimeStep();

	QPointF getMousePosition();
	void setMousePosition(const QPointF& pos);
	int getMouseClick();
	void setMouseClick(int state);

	quint32 getNextIndex();
	PandaObject* findObject(quint32 objectIndex);
	BaseData* findData(quint32 objectIndex, const QString& dataName);

	virtual void update();
	virtual void setDirtyValue();

	const QImage& getRenderedImage();

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
	QImage renderedImage;
	Layer* defaultLayer;

	Data<QPointF> renderSize;
	Data<QColor> backgroundColor;
	Data<double> animTime, timestep;
	Data<QPointF> mousePosition;
	Data<int> mouseClick;

	QPointF mousePositionBuffer;
	int mouseClickBuffer;

	bool animPlaying;
	QTimer* animTimer;

	QMap<QString, QString> groupsMap;

	bool getGroupDescription(const QString &fileName, QString& description);

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
