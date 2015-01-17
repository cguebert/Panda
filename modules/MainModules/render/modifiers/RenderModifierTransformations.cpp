#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>
#include <panda/helper/gl.h>

#include <cmath>

namespace panda {

using types::Point;

class RenderModifier_Translation : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Translation, Renderer)

	RenderModifier_Translation(PandaDocument* parent)
		: Renderer(parent)
		, translation(initData(&translation, "translation", "Translation to apply for the next objects"))
	{
		addInput(translation);
	}

	void render()
	{
		const Point tr = translation.getValue();
		glTranslateReal(tr.x, tr.y, 0);

		QMatrix4x4& mvpMatrix = getMVPMatrix();
		mvpMatrix.translate(tr.x, tr.y);
	}

protected:
	Data<Point> translation;
};

int RenderModifier_TranslationClass = RegisterObject<RenderModifier_Translation>("Render/Modifier/Translation").setDescription("Add a translation transformation");

//****************************************************************************//

class RenderModifier_Rotation : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Rotation, Renderer)

	RenderModifier_Rotation(PandaDocument* parent)
		: Renderer(parent)
		, rotation(initData(&rotation, "rotation", "Rotation to apply for the next objects"))
	{
		addInput(rotation);
	}

	void render()
	{
		glRotateReal(rotation.getValue(), 0, 0, -1);

		QMatrix4x4& mvpMatrix = getMVPMatrix();
		mvpMatrix.rotate(rotation.getValue(), 0, 0, -1);
	}

protected:
	Data<PReal> rotation;
};

int RenderModifier_RotationClass = RegisterObject<RenderModifier_Rotation>("Render/Modifier/Rotation").setDescription("Add a rotation transformation");

//****************************************************************************//

class RenderModifier_Scale : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Scale, Renderer)

	RenderModifier_Scale(PandaDocument* parent)
		: Renderer(parent)
		, scale(initData(&scale, Point(1.0, 1.0), "scaling", "Scaling to apply for the next objects"))
	{
		addInput(scale);
	}

	void render()
	{
		const Point& sc = scale.getValue();
		glScaleReal(sc.x, sc.y, 1);

		QMatrix4x4& mvpMatrix = getMVPMatrix();
		mvpMatrix.scale(sc.x, sc.y, 1);
	}

protected:
	Data<Point> scale;
};

int RenderModifier_ScaleClass = RegisterObject<RenderModifier_Scale>("Render/Modifier/Scale").setDescription("Add a scale transformation");


} // namespace panda
