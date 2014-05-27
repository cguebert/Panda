#ifndef LAYER_H
#define LAYER_H

#include <panda/Dockable.h>
#include <panda/types/ImageWrapper.h>
#include <vector>
#include <QMatrix4x4>

class QOpenGLFramebufferObject;

namespace panda
{

class Renderer;

class BaseDrawTarget
{
public:
	virtual QMatrix4x4& getMVPMatrix() = 0;
};

class BaseLayer : public BaseDrawTarget
{
public:
	virtual void updateLayer(PandaDocument* doc);
	virtual void mergeLayer();

	typedef std::vector<Renderer*> RenderersList;
	virtual RenderersList getRenderers() = 0;

	virtual QString getLayerName() const = 0;
	virtual void setLayerName(QString name) = 0;

	virtual int getCompositionMode() const = 0;
	virtual void setCompositionMode(int mode) = 0;

	virtual PReal getOpacity() const = 0;
	virtual void setOpacity(PReal opa) = 0;

	virtual Data<types::ImageWrapper>* getImage() = 0;

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

	virtual RenderersList getRenderers();

	virtual QString getLayerName() const;
	virtual void setLayerName(QString name);

	virtual int getCompositionMode() const;
	virtual void setCompositionMode(int mode);

	virtual PReal getOpacity() const;
	virtual void setOpacity(PReal opa);

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
