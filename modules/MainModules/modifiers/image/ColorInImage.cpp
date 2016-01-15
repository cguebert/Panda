#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <panda/types/Color.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Rect.h>

#include <panda/graphics/Image.h>

#include <cmath>

namespace panda {

using types::Color;
using types::ImageWrapper;
using types::Point;
using types::Rect;
using graphics::PointInt;

class ModifierImage_GetPixel : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_GetPixel, PandaObject)

	ModifierImage_GetPixel(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData("image", "The image from which to get the color"))
		, position(initData("position", "Position in the image where to get the color"))
		, color(initData("color", "Color extracted from the image"))
	{
		addInput(image);
		addInput(position);

		addOutput(color);
	}

	void update()
	{
		const auto& img = image.getValue().getImage();
		const std::vector<Point>& pos = position.getValue();
		auto colorsList = color.getAccessor();

		int nb = pos.size();
		colorsList.resize(nb);
		for(int i=0; i<nb; ++i)
		{
			const Point p = pos[i];
			const PointInt pt(std::floor(p.x), std::floor(p.y));
			if(img.valid(pt))
				colorsList[i] = Color::fromByte(img.pixel(pt));
			else
				colorsList[i] = Color::null();
		}

		cleanDirty();
	}

protected:
	Data< ImageWrapper > image;
	Data< std::vector<Point> > position;
	Data< std::vector<Color> > color;
};

int ModifierImage_GetPixelClass = RegisterObject<ModifierImage_GetPixel>("Modifier/Image/Get pixel").setDescription("Get colors in specific places in an image");

//****************************************************************************//

class ModifierImage_SetPixel : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_SetPixel, PandaObject)

	ModifierImage_SetPixel(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData("image", "The image in which to modify the pixels"))
		, position(initData("position", "Position in the image to modify"))
		, color(initData("color", "Color to insert in the image"))
		, result(initData("result", "The modified image"))
	{
		addInput(image);
		addInput(position);
		addInput(color);

		addOutput(result);
	}

	void update()
	{
		const auto img = image.getValue().getImage();
		const std::vector<Point>& pos = position.getValue();
		const std::vector<Color>& col = color.getValue();

		graphics::Image tmp = img.clone();

		int nbP = pos.size();
		int nbC = col.size();

		if(nbC < nbP) nbC = 1;

		for(int i=0; i<nbP; ++i)
		{
			const Point p = pos[i];
			const PointInt pt(std::floor(p.x), std::floor(p.y));
			if(tmp.valid(pt))
				tmp.setPixel(pt, col[i%nbC].toHex());
		}

		result.getAccessor()->setImage(tmp);

		cleanDirty();
	}

protected:
	Data< ImageWrapper > image, result;
	Data< std::vector<Point> > position;
	Data< std::vector<Color> > color;
};

int ModifierImage_SetPixelClass = RegisterObject<ModifierImage_SetPixel>("Modifier/Image/Set pixel").setDescription("Set colors in specific places in an image");

//****************************************************************************//

class ModifierImage_ColorInRect : public PandaObject
{
public:
	PANDA_CLASS(ModifierImage_ColorInRect, PandaObject)

	ModifierImage_ColorInRect(PandaDocument *doc)
		: PandaObject(doc)
		, image(initData("image", "The image to analyse"))
		, rectangle(initData("rectangle", "Region to analyse"))
		, color(initData("color", "Mean color in this region of the image"))
	{
		addInput(image);
		addInput(rectangle);

		addOutput(color);
	}

	void update()
	{
		const auto& img = image.getValue().getImage();
		const std::vector<Rect>& rectList = rectangle.getValue();
		auto col = color.getAccessor();

		int nbRects = rectList.size();
		col.resize(nbRects);

		for(int i=0; i<nbRects; ++i)
		{
			Rect rect = rectList[i];
			int x1 = std::max<int>(0, std::floor(rect.left()));
			int y1 = std::max<int>(0, std::floor(rect.top()));
			int x2 = std::min<int>(img.width()-1, std::floor(rect.right()));
			int y2 = std::min<int>(img.height()-1, std::floor(rect.bottom()));

			int nb = 0;
			uint64_t a=0, r=0, g=0, b=0;
			for(int y=y1; y<=y2; ++y)
			{
				const auto line = img.pixel(0, y);
				for(int x=x1; x<=x2; ++x)
				{
					const auto p = &line[x * 4];
					a += graphics::alpha(p);
					r += graphics::red(p);
					g += graphics::green(p);
					b += graphics::blue(p);
					++nb;
				}
			}

			float n = nb * 255;
			if(nb)
				col[i] = Color(r/n, g/n, b/n, a/n);
			else
				col[i] = Color::null();
		}

		cleanDirty();
	}

protected:
	Data< ImageWrapper > image;
	Data< std::vector<Rect> > rectangle;
	Data< std::vector<Color> > color;
};

int ModifierImage_ColorInRectClass = RegisterObject<ModifierImage_ColorInRect>("Modifier/Image/Color in rectangle")
		.setDescription("Computes the mean color inside a given rectangle of an image");

} // namespace Panda
