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

	typedef std::vector<Renderer*> RenderersList;
	virtual RenderersList getRenderers() = 0;

	virtual QString getLayerName() const = 0;
	virtual Data<QString>& getLayerNameData() = 0;

	virtual int getCompositionMode() const = 0;
	virtual Data<int>& getCompositionModeData() = 0;

	virtual PReal getOpacity() const = 0;
	virtual Data<PReal>& getOpacityData() = 0;

	virtual unsigned int getTextureId() const;

	virtual Data<types::ImageWrapper>* getImage() = 0;

protected:
	virtual void iterateRenderers();
	QSharedPointer<QOpenGLFramebufferObject> m_renderFrameBuffer, m_displayFrameBuffer;
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
	virtual Data<QString>& getLayerNameData();

	virtual int getCompositionMode() const;
	virtual Data<int>& getCompositionModeData();

	virtual PReal getOpacity() const;
	virtual Data<PReal>& getOpacityData();

	virtual Data<types::ImageWrapper>* getImage();
	virtual QMatrix4x4& getMVPMatrix();

	virtual void postCreate();

protected:
	Data<QString> m_layerName;
	Data<types::ImageWrapper> m_image;
	Data<int> m_compositionMode;
	Data<PReal> m_opacity;
	QMatrix4x4 m_mvpMatrix;
};

} // namespace panda

#endif // LAYER_H
