#ifndef RENDERER_H
#define RENDERER_H

#include <panda/Dockable.h>
#include <panda/Layer.h>
#include <panda/helper/Gl.h>
#include <QMatrix4x4>

namespace panda
{

class Layer;

class Renderer : public DockableObject
{
public:
	PANDA_CLASS(Renderer, DockableObject)

	explicit Renderer(PandaDocument* doc);
	virtual DockObject* getDefaultDock();

	virtual void render() = 0;

protected:
	QMatrix4x4& getMVPMatrix();
	QSize getLayerSize();
};

} // namespace panda

#endif // RENDERER_H
