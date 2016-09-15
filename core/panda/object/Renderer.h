#ifndef RENDERER_H
#define RENDERER_H

#include <panda/object/Dockable.h>
#include <panda/graphics/Size.h>
#include <panda/graphics/Mat4x4.h>

namespace panda
{

class Layer;
class BaseDrawTarget;
class RenderedDocument;

class PANDA_CORE_API Renderer : public DockableObject
{
public:
	PANDA_CLASS(Renderer, DockableObject)

	explicit Renderer(RenderedDocument* doc);
	virtual DockObject* getDefaultDock() const;

	virtual void render() = 0;

protected:
	graphics::Mat4x4& getMVPMatrix();
	graphics::Size getLayerSize() const;
	BaseDrawTarget* getDrawTarget() const;

	RenderedDocument* m_parentRenderedDocument;
};

} // namespace panda

#endif // RENDERER_H
