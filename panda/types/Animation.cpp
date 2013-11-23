#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

#include <ui/SimpleDataWidget.h>

namespace panda
{

namespace types
{

template<>
QColor interpolate(const QColor& v1, const QColor& v2, double amt)
{
	double r1, r2, g1, g2, b1, b2, a1, a2;
	v1.getRgbF(&r1, &g1, &b1, &a1);
	v2.getRgbF(&r2, &g2, &b2, &a2);
	double r, g, b, a;
	r = interpolate(r1, r2, amt);
	g = interpolate(g1, g2, amt);
	b = interpolate(b1, b2, amt);
	a = interpolate(a1, a2, amt);
	QColor temp;
	temp.setRgbF(r, g, b, a);
	return temp;
}

template class Animation<double>;
template class Animation<QPointF>;
template class Animation<QColor>;

template class Data< Animation<double> >;
template class Data< Animation<QColor> >;
template class Data< Animation<QPointF> >;

int doubleAnimationDataClass = RegisterData< Animation<double> >();
int colorAnimationDataClass = RegisterData< Animation<QColor> >();
int pointAnimationDataClass = RegisterData< Animation<QPointF> >();

} // namespace types

} // namespace panda
