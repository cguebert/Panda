#ifndef GRADIENT_H
#define GRADIENT_H

#include <QVector>
#include <QColor>

namespace panda
{

namespace types
{

class Gradient
{
public:
	typedef QPair<double, QColor> GradientStop;
	typedef QVector<GradientStop> GradientStops;

	enum Extend { EXTEND_PAD = 0, EXTEND_REPEAT = 1, EXTEND_REFLECT = 2 };

	Gradient();

	void clear();

	void add(double position, QColor color);
	QColor get(double position) const;

	void setExtend(int method);
	int getExtend() const;

	void setStops(GradientStops stopsPoints);
	GradientStops getStops() const;

	friend inline bool operator==(const Gradient& g1, const Gradient& g2)
	{
		return g1.extend == g2.extend && g1.stops == g2.stops;
	}
	friend inline bool operator!=(const Gradient& g1, const Gradient& g2)
	{
		return !(g1 == g2);
	}

	static Gradient interpolate(const Gradient& g1, const Gradient& g2, double amt);

protected:
	double extendPos(double position) const;

	static double interpolate(double v1, double v2, double amt);
	static QColor interpolate(const GradientStop& s1, const GradientStop& s2, double pos);
	static QColor interpolate(const QColor& v1, const QColor& v2, double amt);

	GradientStops stops;
	Extend extend;
};

} // namespace types

} // namespace panda

#endif // GRADIENT_H
