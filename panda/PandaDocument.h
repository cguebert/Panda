#ifndef PANDADOCUMENT_H
#define PANDADOCUMENT_H

#include <panda/PandaObject.h>
#include <QObject>
#include <QColor>
#include <QPointF>
#include <QSize>
#include <QMap>
#include <QImage>

namespace panda {

class Layer;

class PandaDocument : public PandaObject
{
    Q_OBJECT
public:
    typedef QListIterator<PandaObject*> ObjectsIterator;
    typedef QListIterator<Layer*> LayersIterator;

    explicit PandaDocument(QObject *parent = 0);

    bool writeFile(const QString& fileName);
    bool readFile(const QString& fileName);

    QString writeTextDocument();
    bool readTextDocument(QString &text);

    void resetDocument();

    PandaObject* createObject(QString registryName);
    int getNbObjects() const;
    ObjectsIterator getObjectsIterator() const;

    PandaObject* getCurrentSelectedObject();
    void setCurrentSelectedObject(PandaObject* object);
    bool isSelected(PandaObject* object) const;
    int getNbSelected() const;
    ObjectsIterator getSelectionIterator() const;

    QColor getBackgroundColor();
    void setBackgroundColor(QColor color);
    QSize getRenderSize();

    double getAnimationTime();
	double getTimeStep();

    quint32 getNextIndex();
    PandaObject* findObject(quint32 objectIndex);
    BaseData* findData(quint32 objectIndex, const QString& dataName);

    virtual void update();
    virtual void setDirtyValue();

    const QImage& getRenderedImage();

    Layer* getDefaultLayer();

    void doAddObject(PandaObject* object);
    void doRemoveObject(PandaObject* object, bool del=true);

protected:
    QList<PandaObject*> pandaObjects, selectedObjects;
    QMap<quint32, PandaObject*> pandaObjectsMap;
    quint32 currentIndex;
    QImage renderedImage;
    Layer* defaultLayer;
    QList<Layer*> layers;
    Data<QPointF> renderSize;
    Data<QColor> backgroundColor;
    Data<double> animTime, timestep;
    bool animPlaying;
    QTimer* animTimer;

private:
    static const quint32 pandaMagicNumber = 0x574f306b;
    static const quint32 pandaVersion = 40;

signals:
    void modified();
    void modifiedObject(panda::PandaObject*);
    void dirtyObject(panda::PandaObject*);
    void addedObject(panda::PandaObject*);
    void removedObject(panda::PandaObject*);
    void savingObject(QDataStream&, panda::PandaObject*);
    void savingObject(QTextStream&, panda::PandaObject*);
    void loadingObject(QDataStream&, panda::PandaObject*);
    void loadingObject(QTextStream&, panda::PandaObject*);
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
};

} // namespace panda

#endif // PANDADOCUMENT_H
