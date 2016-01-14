#include <panda/types/Color.h>

#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/data/DataFactory.h>

#include <cmath>

namespace
{

	float bound(float min, float v, float max)
	{
		if (v < min)
			return min;
		if (v > max)
			return max;
		return v;
	}

}

namespace panda
{

namespace types
{

Color Color::bounded() const
{
	return Color(
				bound(0.0f, r, 1.0f),
				bound(0.0f, g, 1.0f),
				bound(0.0f, b, 1.0f),
				bound(0.0f, a, 1.0f));
}

void Color::getHsv(float& hue, float& sat, float& val, float& alpha) const
{
	alpha = a;

	float max = std::max(r, std::max(g, b));
	float min = std::min(r, std::min(g, b));

	float range = max - min;
	val = max;
	sat = 0.0f;
	hue = 0.0f;

	if(max != 0)
		sat = range / max;

	if(sat != 0)
	{
		float h;

		if(r == max)
			h = (g - b) / range;
		else if(g == max)
			h = 2 + (b - r) / range;
		else
			h = 4 + (r - g) / range;

		hue = h / 6.0f;

		if(hue < 0.0f)
			hue += 1.0f;
	}
}

Color Color::fromHsv(float hue, float sat, float val, float alpha)
{
	if(hue == 1)
		hue = 0;
	else
		hue *= 6;

	int i = static_cast<int>(std::floor(hue));
	float f = hue - i;
	float p = val * (1 - sat);
	float q = val * (1 - (sat * f) );
	float t = val * (1 - (sat * (1 - f)));

	float r = 0.0f, g = 0.0f, b = 0.0f;

	switch( i )
	{
		case 0: r = val; g = t;   b = p; break;
		case 1: r = q;   g = val; b = p; break;
		case 2: r = p;   g = val; b = t; break;
		case 3: r = p;   g = q;   b = val; break;
		case 4: r = t;   g = p;   b = val; break;
		case 5: r = val; g = p;   b = q; break;
	}

	return Color(r, g, b, alpha);
}

uint32_t Color::toHex() const
{
	uint8_t alpha = static_cast<uint8_t>(a * 0xFF);
	uint8_t red = static_cast<uint8_t>(r * 0xFF);
	uint8_t green = static_cast<uint8_t>(g * 0xFF);
	uint8_t blue = static_cast<uint8_t>(b * 0xFF);
	return  (alpha & 0xFF) << 24 |
			(red   & 0xFF) << 16 |
			(green & 0xFF) << 8  |
			(blue  & 0xFF);
}

Color Color::fromHex(uint32_t hexValue)
{
	uint8_t alpha = (hexValue >> 24) & 0xFF;
	uint8_t red = (hexValue >> 16) & 0xFF;
	uint8_t green = (hexValue >> 8) & 0xFF;
	uint8_t blue = hexValue & 0xFF;
	return Color(red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);
}

Color Color::fromByte(const unsigned char* px)
{
	return Color(px[2] / 255.0f, px[1] / 255.0f, px[0] / 255.0f, px[4] / 255.0f);
}

//****************************************************************************//

template<> PANDA_CORE_API std::string DataTrait<Color>::valueTypeName() { return "color"; }

template<>
PANDA_CORE_API void DataTrait<Color>::writeValue(XmlElement& elem, const Color& v)
{
	elem.setAttribute("r", v.r);
	elem.setAttribute("g", v.g);
	elem.setAttribute("b", v.b);
	elem.setAttribute("a", v.a);
}

template<>
PANDA_CORE_API void DataTrait<Color>::readValue(XmlElement& elem, Color& v)
{
	Color c;
	c.r = elem.attribute("r").toFloat();
	c.g = elem.attribute("g").toFloat();
	c.b = elem.attribute("b").toFloat();
	c.a = elem.attribute("a").toFloat();
	v = c.bounded();
}

//****************************************************************************//

template class PANDA_CORE_API Data<Color>;
template class PANDA_CORE_API Data< std::vector<Color> >;

template class PANDA_CORE_API Animation<Color>;
template class PANDA_CORE_API Data< Animation<Color> >;

int colorDataClass = RegisterData< Color >();
int colorVectorDataClass = RegisterData< std::vector<Color> >();
int colorAnimationDataClass = RegisterData< Animation<Color> >();

} // namespace types

} // namespace panda
