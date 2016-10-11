#include <panda/document/RenderedDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/document/DocumentRenderer.h>
#include <panda/document/Scheduler.h>
#include <panda/object/Layer.h>
#include <panda/object/Renderer.h>
#include <panda/helper/GradientCache.h>
#include <panda/helper/ShaderCache.h>

namespace panda {

using types::Color;
using types::Point;

RenderedDocument::RenderedDocument(gui::BaseGUI& gui)
	: PandaDocument(gui)
	, m_renderSize(initData(Point(800,600), "render size", "Size of the image to be rendered"))
	, m_backgroundColor(initData(Color::white(), "background color", "Background color of the image to be rendered"))
	, m_renderer(std::make_unique<DocumentRenderer>(*this))
{
	addInput(m_renderSize);
	addInput(m_backgroundColor);

	m_defaultLayer = std::make_shared<Layer>(this);
	m_defaultLayer->getLayerNameData().setValue("Default Layer");
}

RenderedDocument::~RenderedDocument() = default;

graphics::Size RenderedDocument::getRenderSize() const
{
	Point pt = m_renderSize.getValue();
	return graphics::Size(std::max(1, static_cast<int>(pt.x)), std::max(1, static_cast<int>(pt.y)));
}

void RenderedDocument::setRenderSize(graphics::Size size)
{ 
	m_renderSize.setValue(
		panda::types::Point(
			std::max(1.f, static_cast<float>(size.width())), 
			std::max(1.f, static_cast<float>(size.height()))
			)
		); 
}

void RenderedDocument::update()
{
	helper::GradientCache::getInstance()->resetUsedFlag();
	helper::ShaderCache::getInstance()->resetUsedFlag();

	if (!m_renderer->renderingMainView()) // If it not already the case, make the OpenGL context current
	{
		helper::ScopedEvent log("context make current");
		m_gui.contextMakeCurrent();
	}

	PandaDocument::update();

	m_renderer->renderGL();

	helper::GradientCache::getInstance()->clearUnused();
	helper::ShaderCache::getInstance()->clearUnused();

	if (!m_renderer->renderingMainView()) // Release the context if we made it current ourselves
	{
		helper::ScopedEvent log("context done current");
		m_gui.contextDoneCurrent();
	}
}

graphics::Framebuffer& RenderedDocument::getFBO()
{
	updateIfDirty();
	return m_renderer->getFBO();
}

} // namespace panda


