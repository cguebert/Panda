#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Rect.h>
#include <cmath>

namespace panda {

using types::Point;
using types::Rect;
using types::ImageWrapper;

class ModifierImage_GetPixel : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_GetPixel, PandaObject)

	ModifierImage_GetPixel(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData(&image, "image", "The image from which to get the color"))
		, position(initData(&position, "position", "Position in the image where to get the color"))
		, color(initData(&color, "color", "Color extracted from the image"))
	{
		addInput(&image);
		addInput(&position);

		addOutput(&color);
	}

	void update()
	{
		const QImage& img = image.getValue().getImage();
		const QVector<Point>& pos = position.getValue();
		auto colorsList = color.getAccessor();

		int nb = pos.size();
		colorsList.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const Point p = pos[i];
			const QPoint pt = QPoint(std::floor(p.x), std::floor(p.y));
			if(img.valid(pt))
				colorsList[i] = img.pixel(pt);
			else
				colorsList[i] = QColor();
		}

		cleanDirty();
	}

protected:
	Data< ImageWrapper > image;
	Data< QVector<Point> > position;
	Data< QVector<QColor> > color;
};

int ModifierImage_GetPixelClass = RegisterObject<ModifierImage_GetPixel>("Modifier/Image/Get pixel").setDescription("Get colors in specific places in an image");

//*************************************************************************//

class ModifierImage_SetPixel : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_SetPixel, PandaObject)

	ModifierImage_SetPixel(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData(&image, "image", "The image in which to modify the pixels"))
		, position(initData(&position, "position", "Position in the image to modify"))
		, color(initData(&color, "color", "Color to insert in the image"))
		, result(initData(&result, "result", "The modified image"))
	{
		addInput(&image);
		addInput(&position);
		addInput(&color);

		addOutput(&result);
	}

	void update()
	{
		const QImage& img = image.getValue().getImage();
		const QVector<Point>& pos = position.getValue();
		const QVector<QColor>& col = color.getValue();

		QImage tmp = img;

		int nbP = pos.size();
		int nbC = col.size();

		if(nbC < nbP) nbC = 1;

		for(int i=0; i<nbP; ++i)
		{
			const Point p = pos[i];
			const QPoint pt = QPoint(std::floor(p.x), std::floor(p.y));
			if(tmp.valid(pt))
				tmp.setPixel(pt, col[i%nbC].rgba());
		}

		result.getAccessor()->setImage(tmp);

		cleanDirty();
	}

protected:
	Data< ImageWrapper > image, result;
	Data< QVector<Point> > position;
	Data< QVector<QColor> > color;
};

int ModifierImage_SetPixelClass = RegisterObject<ModifierImage_SetPixel>("Modifier/Image/Set pixel").setDescription("Set colors in specific places in an image");

//*************************************************************************//

class ModifierImage_ColorInRect : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_ColorInRect, PandaObject)

	ModifierImage_ColorInRect(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData(&image, "image", "The image to analyse"))
		, rectangle(initData(&rectangle, "rectangle", "Region to analyse"))
		, color(initData(&color, "color", "Mean color in this region of the image"))
	{
		addInput(&image);
		addInput(&rectangle);

		addOutput(&color);
	}

	void update()
	{
		const QImage& img = image.getValue().getImage();
		const QVector<Rect>& rectList = rectangle.getValue();
		auto col = color.getAccessor();

		if(img.format() != QImage::Format_ARGB32)
		{
			col.clear();
			return;
		}

		int nb = rectList.size();
		col.resize(nb);

		for(int i=0; i<nb; ++i)
		{
			Rect rect = rectList[i];
			int x1 = qMax<int>(0, std::floor(rect.left()));
			int y1 = qMax<int>(0, std::floor(rect.top()));
			int x2 = qMin<int>(img.width()-1, std::floor(rect.right()));
			int y2 = qMin<int>(img.height()-1, std::floor(rect.bottom()));

			int nb = 0;
			ulong a=0, r=0, g=0, b=0;
			for(int y=y1; y<=y2; ++y)
			{
				const QRgb* line = reinterpret_cast<const QRgb*>(img.scanLine(y));
				for(int x=x1; x<=x2; ++x)
				{
					QRgb p = line[x];
					a += qAlpha(p);
					r += qRed(p);
					g += qGreen(p);
					b += qBlue(p);
					++nb;
				}
			}

			if(nb)
				col[i] = QColor(r/nb, g/nb, b/nb, a/nb);
			else
				col[i] = QColor();
		}

		cleanDirty();
	}

protected:
	Data< ImageWrapper > image;
	Data< QVector<Rect> > rectangle;
	Data< QVector<QColor> > color;
};

int ModifierImage_ColorInRectClass = RegisterObject<ModifierImage_ColorInRect>("Modifier/Image/Color in rectangle")
		.setDescription("Computes the mean color inside a given rectangle of an image");

} // namespace Panda
