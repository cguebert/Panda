#ifndef GRADIENT_H
#define GRADIENT_H

#include <panda/helper/system/Config.h>
#include <QVector>
#include <QColor>

namespace panda
{

namespace types
{

class Gradient
{
public:
	typedef QPair<PReal, QColor> GradientStop;
	typedef QVector<GradientStop> GradientStops;

	enum Extend { EXTEND_PAD = 0, EXTEND_REPEAT = 1, EXTEND_REFLECT = 2 };

	Gradient();

	void clear();

	void add(PReal position, QColor color);
	QColor get(PReal position) const;

	void setExtend(int method);
	int getExtend() const;

	void setStops(GradientStops stopsPoints);
	GradientStops getStops() const;
	GradientStops getStopsForEdit() const; // this one doesn't lie

	friend inline bool operator==(const Gradient& g1, const Gradient& g2)
	{
		return g1.extend == g2.extend && g1.stops == g2.stops;
	}
	friend inline bool operator!=(const Gradient& g1, const Gradient& g2)
	{
		return !(g1 == g2);
	}

	static Gradient interpolate(const Gradient& g1, const Gradient& g2, PReal amt);
	static QColor interpolate(const GradientStop& s1, const GradientStop& s2, PReal pos);
	static QColor interpolate(const QColor& v1, const QColor& v2, PReal amt);

protected:
	PReal extendPos(PReal position) const;

	GradientStops stops;
	Extend extend;
};

} // namespace types

} // namespace panda

#endif // GRADIENT_H
