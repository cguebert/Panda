#ifndef RENDERER_H
#define RENDERER_H

#include <panda/Dockable.h>
#include <panda/helper/gl.h>
#include <QMatrix4x4>

namespace panda
{

class Layer;
class BaseDrawTarget;

class Renderer : public DockableObject
{
public:
	PANDA_CLASS(Renderer, DockableObject)

	explicit Renderer(PandaDocument* doc);
	virtual DockObject* getDefaultDock() const;

	virtual void render() = 0;

protected:
	QMatrix4x4& getMVPMatrix();
	QSize getLayerSize() const;
	BaseDrawTarget* getDrawTarget() const;
};

} // namespace panda

#endif // RENDERER_H
