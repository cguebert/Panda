#ifndef LAYER_H
#define LAYER_H

#include <panda/object/Dockable.h>
#include <panda/types/ImageWrapper.h>
#include <panda/graphics/Mat4x4.h>

namespace panda
{

class RenderedDocument;
class Renderer;

namespace graphics
{
	class Framebuffer;
}

class BaseDrawTarget
{
public:
	virtual graphics::Mat4x4& getMVPMatrix() = 0;
	virtual graphics::Size getLayerSize() const = 0;
};

class PANDA_CORE_API BaseLayer : public BaseDrawTarget
{
public:
	virtual void updateLayer(RenderedDocument* doc);

	typedef std::vector<Renderer*> RenderersList;
	virtual RenderersList getRenderers() = 0;

	virtual const std::string getLayerName() const = 0;
	virtual Data<std::string>& getLayerNameData() = 0;

	virtual int getCompositionMode() const = 0;
	virtual Data<int>& getCompositionModeData() = 0;

	virtual float getOpacity() const = 0;
	virtual Data<float>& getOpacityData() = 0;

	virtual unsigned int getTextureId() const;

	virtual Data<types::ImageWrapper>* getImage() = 0;

protected:
	virtual void iterateRenderers();
	std::shared_ptr<graphics::Framebuffer> m_renderFrameBuffer, m_displayFrameBuffer;
};

//****************************************************************************//

class PANDA_CORE_API Layer : public DockObject, public BaseLayer
{
public:
	PANDA_CLASS(Layer, DockObject)

	explicit Layer(RenderedDocument* parent = nullptr);

	void update() override;
	bool accepts(DockableObject* dockable) const override;

	RenderersList getRenderers();

	const std::string getLayerName() const override;
	Data<std::string>& getLayerNameData() override;
	std::string getLabel() const override;

	int getCompositionMode() const override;
	Data<int>& getCompositionModeData() override;

	float getOpacity() const override;
	Data<float>& getOpacityData() override;

	Data<types::ImageWrapper>* getImage() override;

	graphics::Mat4x4& getMVPMatrix() override;
	graphics::Size getLayerSize() const override;

	virtual void postCreate();
	virtual void removedFromDocument();

protected:
	RenderedDocument* m_parentRenderedDocument;
	Data<std::string> m_layerName;
	Data<types::ImageWrapper> m_image;
	Data<int> m_compositionMode;
	Data<float> m_opacity;
	graphics::Mat4x4 m_mvpMatrix;
};

//****************************************************************************//

inline const std::string Layer::getLayerName() const
{ return m_layerName.getValue(); }

inline Data<std::string>& Layer::getLayerNameData()
{ return m_layerName; }

inline std::string Layer::getLabel() const
{ return m_layerName.getValue(); }

inline int Layer::getCompositionMode() const
{ return m_compositionMode.getValue(); }

inline Data<int>& Layer::getCompositionModeData()
{ return m_compositionMode; }

inline float Layer::getOpacity() const
{ return m_opacity.getValue(); }

inline Data<float>& Layer::getOpacityData()
{ return m_opacity; }

inline Data<types::ImageWrapper>* Layer::getImage()
{ return &m_image; }

inline graphics::Mat4x4& Layer::getMVPMatrix()
{ return m_mvpMatrix; }

} // namespace panda

#endif // LAYER_H
