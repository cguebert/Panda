#ifndef RENDERER_H
#define RENDERER_H

#include <panda/Dockable.h>
#include <panda/Layer.h>
#include <QPointer>

class QPainter;

namespace panda
{

class Layer;

class Renderer : public DockableObject
{
public:
	explicit Renderer(PandaDocument* doc);
	virtual DockObject* getDefaultDock();
	virtual void setDirtyValue();

	virtual void render(QPainter* /*painter*/) = 0;
};

} // namespace panda

#endif // RENDERER_H
