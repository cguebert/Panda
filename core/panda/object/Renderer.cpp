#include <panda/document/RenderedDocument.h>
#include <panda/object/Renderer.h>
#include <panda/object/Layer.h>

namespace panda
{

Renderer::Renderer(RenderedDocument* doc)
	: DockableObject(doc)
	, m_parentRenderedDocument(doc)
{
	BaseData* data = doc->getData("render size");
	if(data) addInput(*data);
}

DockObject* Renderer::getDefaultDock() const
{
	return m_parentRenderedDocument->getDefaultLayer();
}

graphics::Mat4x4& Renderer::getMVPMatrix()
{
	return getDrawTarget()->getMVPMatrix();
}

graphics::Size Renderer::getLayerSize() const
{
	return getDrawTarget()->getLayerSize();
}

BaseDrawTarget* Renderer::getDrawTarget() const
{
	BaseDrawTarget* drawTarget = dynamic_cast<BaseDrawTarget*>(getParentDock());
	if(drawTarget)
		return drawTarget;

	const auto outputs = getOutputs();
	for(auto output : outputs)
	{
		drawTarget = dynamic_cast<BaseDrawTarget*>(output);
		if(drawTarget)
			return drawTarget;
	}

	return nullptr;
}

} // namespace panda
