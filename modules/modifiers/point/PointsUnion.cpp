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
		, outputIndices(initData(&outputIndices, "indices", "Indices of the valid points"))
	{
		addInput(&image);
		addInput(&inputPoints);

		addOutput(&outputPoints);
		addOutput(&outputIndices);
	}

	void update()
	{
		const QImage& img = image.getValue();

		const QVector<QPointF>& inPts = inputPoints.getValue();
		auto outPts = outputPoints.getAccessor();
		auto outId = outputIndices.getAccessor();
		int nb = inPts.size();
		outPts.clear();
		outId.clear();

		if(image.getParent() && !img.isNull())
		{
			for(int i=0; i<nb; ++i)
			{
				QPointF pt = inPts[i];
				QPoint iPt(qRound(pt.x()), qRound(pt.y()));
				QRgb color = img.pixel(iPt);
				if(qGray(color) > 128)
				{
					outPts.push_back(pt);
					outId.push_back(i);
				}
			}
		}

		this->cleanDirty();
	}

protected:
	Data<QImage> image;
	Data< QVector<QPointF> > inputPoints, outputPoints;
	Data< QVector<int> > outputIndices;
};

int ModifierPoints_UnionClass = RegisterObject<ModifierPoints_Union>("Modifier/Point/Union").setName("Points union").setDescription("Remove points that are in dark areas of the image");

} // namespace Panda
