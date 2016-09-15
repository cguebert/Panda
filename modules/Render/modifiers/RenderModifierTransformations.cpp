#include <GL/glew.h>

#include <panda/document/RenderedDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/types/Point.h>

#include <cmath>

namespace panda {

using types::Point;

class RenderModifier_Translation : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Translation, Renderer)

	RenderModifier_Translation(RenderedDocument* parent)
		: Renderer(parent)
		, translation(initData("translation", "Translation to apply for the next objects"))
	{
		addInput(translation);
	}

	void render()
	{
		auto& mvpMatrix = getMVPMatrix();
		const Point tr = translation.getValue();
		mvpMatrix.translate(tr.x, tr.y, 0);
	}

protected:
	Data<Point> translation;
};

int RenderModifier_TranslationClass = RegisterObject<RenderModifier_Translation, RenderedDocument>("Render/Modifier/Translation").setDescription("Add a translation transformation");

//****************************************************************************//

class RenderModifier_Rotation : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Rotation, Renderer)

	RenderModifier_Rotation(RenderedDocument* parent)
		: Renderer(parent)
		, rotation(initData("rotation", "Rotation to apply for the next objects"))
	{
		addInput(rotation);
	}

	void render()
	{
		auto& mvpMatrix = getMVPMatrix();
		mvpMatrix.rotate(rotation.getValue(), 0, 0, -1);
	}

protected:
	Data<float> rotation;
};

int RenderModifier_RotationClass = RegisterObject<RenderModifier_Rotation, RenderedDocument>("Render/Modifier/Rotation").setDescription("Add a rotation transformation");

//****************************************************************************//

class RenderModifier_Scale : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Scale, Renderer)

	RenderModifier_Scale(RenderedDocument* parent)
		: Renderer(parent)
		, scale(initData(Point(1.0, 1.0), "scaling", "Scaling to apply for the next objects"))
	{
		addInput(scale);
	}

	void render()
	{
		auto& mvpMatrix = getMVPMatrix();
		const Point& sc = scale.getValue();
		mvpMatrix.scale(sc.x, sc.y, 1);
	}

protected:
	Data<Point> scale;
};

int RenderModifier_ScaleClass = RegisterObject<RenderModifier_Scale, RenderedDocument>("Render/Modifier/Scale").setDescription("Add a scale transformation");


} // namespace panda
