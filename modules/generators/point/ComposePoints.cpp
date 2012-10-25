#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

namespace panda {

class GeneratorPoints_Compose : public PandaObject
{
public:
	GeneratorPoints_Compose(PandaDocument *doc)
		: PandaObject(doc)
		, ptX(initData(&ptX, "x", "Horizontal position of the point"))
		, ptY(initData(&ptY, "y", "Vertical position of the point"))
		, point(initData(&point, "point", "Point created from the 2 coordinates"))
	{
		addInput(&ptX);
		addInput(&ptY);

		addOutput(&point);
	}

	void update()
	{
		point.setValue(QPointF(ptX.getValue(), ptY.getValue()));

		this->cleanDirty();
	}

protected:
	Data<double> ptX, ptY;
	Data<QPointF> point;
};

int GeneratorPoints_ComposeClass = RegisterObject("Generator/Point/Create point").setName("Reals to point").setClass<GeneratorPoints_Compose>().setDescription("Create a point from 2 reals");

class GeneratorPoints_Decompose : public PandaObject
{
public:
	GeneratorPoints_Decompose(PandaDocument *doc)
		: PandaObject(doc)
		, ptX(initData(&ptX, "x", "Horizontal position of the point"))
		, ptY(initData(&ptY, "y", "Vertical position of the point"))
		, point(initData(&point, "point", "Point from which to extract the 2 coordinates"))
	{
		addInput(&point);

		addOutput(&ptX);
		addOutput(&ptY);
	}

	void update()
	{
		QPointF pt = point.getValue();
		ptX.setValue(pt.x());
		ptY.setValue(pt.y());

		this->cleanDirty();
	}

protected:
	Data<double> ptX, ptY;
	Data<QPointF> point;
};

int GeneratorPoints_DecomposeClass = RegisterObject("Generator/Point/Decompose point").setName("Point to reals").setClass<GeneratorPoints_Decompose>().setDescription("Extract the coordinates of a point");


} // namespace Panda
