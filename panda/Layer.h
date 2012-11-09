#ifndef LAYER_H
#define LAYER_H

#include <panda/Dockable.h>
#include <QList>
#include <QImage>

class QPainter;

namespace panda
{

class Renderer;

class Layer : public DockObject
{
public:
    explicit Layer(PandaDocument *parent = 0);

    virtual void update();
    virtual void mergeLayer(QPainter* docPainter);
	virtual bool accepts(DockableObject* dockable) const;

	virtual QString getLayerName();
	virtual void setLayerName(QString name);

	virtual int getCompositionMode();
	virtual void setCompositionMode(int mode);

	virtual double getOpacity();
	virtual void setOpacity(double opa);

	virtual void postCreate(PandaDocument* doc);

protected:
	Data<QString> layerName;
    Data<QImage> image;
    Data<int> compositionMode;
    Data<double> opacity;
};

} // namespace panda

#endif // LAYER_H
