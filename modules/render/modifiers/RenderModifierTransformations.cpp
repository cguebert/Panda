#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Renderer.h>

#include <QtGui/qopengl.h>

#define _USE_MATH_DEFINES
#include <math.h>

namespace panda {

class RenderModifier_Translation : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Translation, Renderer)

	RenderModifier_Translation(PandaDocument *parent)
		: Renderer(parent)
		, translation(initData(&translation, "translation", "Translation to apply for the next objects"))
	{
		addInput(&translation);
	}

	void render()
	{
		const QPointF tr = translation.getValue();
		glTranslated(tr.x(), tr.y(), 0);
	}

protected:
	Data<QPointF> translation;
};

int RenderModifier_TranslationClass = RegisterObject<RenderModifier_Translation>("Render/Modifier/Translation").setDescription("Add a translation transformation");

//*************************************************************************//

class RenderModifier_Rotation : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Rotation, Renderer)

	RenderModifier_Rotation(PandaDocument *parent)
		: Renderer(parent)
		, rotation(initData(&rotation, "rotation", "Rotation to apply for the next objects"))
	{
		addInput(&rotation);
	}

	void render()
	{
		glRotated(rotation.getValue(), 0, 0, -1);
	}

protected:
	Data<double> rotation;
};

int RenderModifier_RotationClass = RegisterObject<RenderModifier_Rotation>("Render/Modifier/Rotation").setDescription("Add a rotation transformation");

//*************************************************************************//

class RenderModifier_Scale : public Renderer
{
public:
	PANDA_CLASS(RenderModifier_Scale, Renderer)

	RenderModifier_Scale(PandaDocument *parent)
		: Renderer(parent)
		, scale(initData(&scale, QPointF(1.0, 1.0), "scaling", "Scaling to apply for the next objects"))
	{
		addInput(&scale);
	}

	void render()
	{
		const QPointF& sc = scale.getValue();
		glScaled(sc.x(), sc.y(), 1);
	}

protected:
	Data<QPointF> scale;
};

int RenderModifier_ScaleClass = RegisterObject<RenderModifier_Scale>("Render/Modifier/Scale").setDescription("Add a scale transformation");


} // namespace panda