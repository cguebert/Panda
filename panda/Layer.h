#ifndef LAYER_H
#define LAYER_H

#include <panda/Dockable.h>
#include <QList>
#include <QImage>
#include <QOpenGLFramebufferObject>

class QPainter;

namespace panda
{

class Renderer;

class BaseLayer
{
public:
	virtual void updateLayer(PandaDocument* doc);
	virtual void mergeLayer(QPainter* docPainter);

	virtual void updateLayerOpenGL(PandaDocument* doc);
	virtual void mergeLayerOpenGL();

	virtual QList<Renderer*> getRenderers() = 0;

	virtual QString getLayerName() const = 0;
	virtual void setLayerName(QString name) = 0;

	virtual int getCompositionMode() const = 0;
	virtual void setCompositionMode(int mode) = 0;

	virtual double getOpacity() const = 0;
	virtual void setOpacity(double opa) = 0;

	virtual Data<QImage>* getImage() = 0;
};

class Layer : public DockObject, public BaseLayer
{
public:
	PANDA_CLASS(Layer, DockObject)

	explicit Layer(PandaDocument *parent = 0);

	virtual void update();
	virtual bool accepts(DockableObject* dockable) const;

	virtual QList<Renderer*> getRenderers();

	virtual QString getLayerName() const;
	virtual void setLayerName(QString name);

	virtual int getCompositionMode() const;
	virtual void setCompositionMode(int mode);

	virtual double getOpacity() const;
	virtual void setOpacity(double opa);

	virtual Data<QImage>* getImage();

	virtual void postCreate();

protected:
	QSharedPointer<QOpenGLFramebufferObject> frameBuffer;
	Data<QString> layerName;
	Data<QImage> image;
	Data<int> compositionMode;
	Data<double> opacity;
};

} // namespace panda

#endif // LAYER_H
