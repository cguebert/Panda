#include <panda/Renderer.h>
#include <panda/PandaDocument.h>
#include <panda/Layer.h>

namespace panda
{

Renderer::Renderer(PandaDocument* doc)
	: DockableObject(doc)
{
}

void Renderer::setDirtyValue()
{
	DataNode::setDirtyValue();
	emit dirty(this);
}

DockObject* Renderer::getDefaultDock(PandaDocument* doc)
{
	return doc->getDefaultLayer();
}

} // namespace panda
