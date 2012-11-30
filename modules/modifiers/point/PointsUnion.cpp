#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QPointF>

namespace panda {

class ModifierPoints_Union : public PandaObject
{
public:
	PANDA_CLASS(ModifierPoints_Union, PandaObject)

	ModifierPoints_Union(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData(&image, "image", "Image used to select valid points"))
		, inputPoints(initData(&inputPoints, "input", "List of points to test"))
		, outputPoints(initData(&outputPoints, "output", "List of valid points" ))
	{
		addInput(&image);
		addInput(&inputPoints);

		addOutput(&outputPoints);

		image.setDisplayed(false);
	}

	void update()
	{
		const QImage& img = image.getValue();

		const QVector<QPointF>& inPts = inputPoints.getValue();
		QVector<QPointF>& outPts = *outputPoints.beginEdit();
		int nb = inPts.size();
		outPts.clear();

		if(image.getParent() && !img.isNull())
		{
			for(int i=0; i<nb; ++i)
			{
				QPointF pt = inPts[i];
				QPoint iPt(qRound(pt.x()), qRound(pt.y()));
				QRgb color = img.pixel(iPt);
				if(qGray(color) > 128)
					outPts.append(pt);
			}
		}

		outputPoints.endEdit();
		this->cleanDirty();
	}

protected:
	Data<QImage> image;
	Data< QVector<QPointF> > inputPoints, outputPoints;
};

int ModifierPoints_UnionClass = RegisterObject("Modifier/Point/Union").setClass<ModifierPoints_Union>().setName("Points union").setDescription("Remove points that are in dark areas of the image");

} // namespace Panda
