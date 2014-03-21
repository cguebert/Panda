#ifndef LAYER_H
#define LAYER_H

#include <panda/Dockable.h>
#include <panda/types/ImageWrapper.h>
#include <QList>
#include <QMatrix4x4>

class QOpenGLFramebufferObject;

namespace panda
{

class Renderer;

class BaseLayer
{
public:
	virtual void updateLayer(PandaDocument* doc);
	virtual void mergeLayer();

	virtual QList<Renderer*> getRenderers() = 0;

	virtual QString getLayerName() const = 0;
	virtual void setLayerName(QString name) = 0;

	virtual int getCompositionMode() const = 0;
	virtual void setCompositionMode(int mode) = 0;

	virtual double getOpacity() const = 0;
	virtual void setOpacity(double opa) = 0;

	virtual Data<types::ImageWrapper>* getImage() = 0;
	virtual QMatrix4x4& getMVPMatrix() = 0;

protected:
	virtual void iterateRenderers();
	QSharedPointer<QOpenGLFramebufferObject> renderFrameBuffer, displayFrameBuffer;
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

	virtual Data<types::ImageWrapper>* getImage();
	virtual QMatrix4x4& getMVPMatrix();

	virtual void postCreate();

protected:
	Data<QString> layerName;
	Data<types::ImageWrapper> image;
	Data<int> compositionMode;
	Data<PReal> opacity;
	QMatrix4x4 mvpMatrix;
};

} // namespace panda

#endif // LAYER_H
