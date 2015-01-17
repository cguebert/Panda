#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>

namespace panda {

class RenderModifier_BlendMode : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_BlendMode, Renderer)

	RenderModifier_BlendMode(PandaDocument* parent)
		: Renderer(parent)
		, source(initData(&source, 4, "source", "Source blending mode"))
		, destination(initData(&destination, 6, "destination", "Destination blending mode"))
	{
		addInput(source);
		addInput(destination);

		source.setWidget("enum");
		source.setWidgetData("Zero;One;Source Color;One Minus Source Color"
							 ";Source Alpha;One Minus Source Alpha"
							 ";Dest Alpha;One Minus Dest Alpha");

		destination.setWidget("enum");
		destination.setWidgetData("Zero;One;Dest Color;One Minus Dest Color;Source Alpha Saturate"
								  ";Source Alpha;One Minus Source Alpha"
								  ";Dest Alpha;One Minus Dest Alpha");
	}

	void render()
	{
		const int srcVals[] = {GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR,
							   GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
							   GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA};
		const int destVals[] = {GL_ZERO, GL_ONE, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR,
								GL_SRC_ALPHA_SATURATE,
								GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
								GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA};
		int src = qBound(0, source.getValue(), 7);
		int dest = qBound(0, destination.getValue(), 8);

		glEnable(GL_BLEND);
		glBlendFunc(srcVals[src], destVals[dest]);
	}

protected:
	Data<int> source, destination;
};

int RenderModifier_BlendModeClass = RegisterObject<RenderModifier_BlendMode>("Render/Modifier/Blend mode").setDescription("Change the blend mode");

} // namespace panda
