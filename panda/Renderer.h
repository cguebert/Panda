#ifndef RENDERER_H
#define RENDERER_H

#include <panda/Dockable.h>
#include <panda/Layer.h>

namespace panda
{

class Layer;

class Renderer : public DockableObject
{
public:
	PANDA_CLASS(Renderer, DockableObject)

	explicit Renderer(PandaDocument* doc);
	virtual DockObject* getDefaultDock();
	virtual void setDirtyValue();

	virtual void render() = 0;
};

} // namespace panda

#endif // RENDERER_H
