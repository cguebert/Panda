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
		const QVector<double>	&xVal = ptX.getValue(),
								&yVal = ptY.getValue();
		QVector<QPointF>& pts = *point.beginEdit();

		int nb = qMin(xVal.size(), yVal.size());
		pts.resize(nb);
		for(int i=0; i<nb; ++i)
			pts[i] = QPointF(xVal[i], yVal[i]);

		point.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<double> > ptX, ptY;
	Data< QVector<QPointF> > point;
};

int GeneratorPoints_ComposeClass = RegisterObject("Generator/Point/Create point").setName("Reals to point").setClass<GeneratorPoints_Compose>().setDescription("Create a point from 2 reals");

//*************************************************************************//

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
		QVector<double>	&xVal = *ptX.beginEdit(),
						&yVal = *ptY.beginEdit();
		const QVector<QPointF>& pts = point.getValue();

		int nb = pts.size();
		xVal.resize(nb);
		yVal.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const QPointF pt = pts[i];
			xVal[i] = pt.x();
			yVal[i] = pt.y();
		}

		ptX.endEdit();
		ptY.endEdit();
		this->cleanDirty();
	}

protected:
	Data< QVector<double> > ptX, ptY;
	Data< QVector<QPointF> > point;
};

int GeneratorPoints_DecomposeClass = RegisterObject("Generator/Point/Decompose point").setName("Point to reals").setClass<GeneratorPoints_Decompose>().setDescription("Extract the coordinates of a point");


} // namespace Panda
