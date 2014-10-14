#ifndef TYPES_GRADIENT_H
#define TYPES_GRADIENT_H

#include <panda/core.h>
#include <panda/types/Color.h>

#include <QPair>
#include <QVector>

namespace panda
{

namespace types
{

class PANDA_CORE_API Gradient
{
public:
	typedef QPair<PReal, types::Color> GradientStop;
	typedef QVector<GradientStop> GradientStops;

	enum Extend { EXTEND_PAD = 0, EXTEND_REPEAT = 1, EXTEND_REFLECT = 2 };

	Gradient();

	void clear();

	void add(PReal position, types::Color color);
	types::Color get(PReal position) const;

	void setExtend(int method);
	int getExtend() const;

	void setStops(GradientStops stopsPoints);
	GradientStops getStops() const;
	GradientStops getStopsForEdit() const; // this one doesn't lie

	friend inline bool operator==(const Gradient& g1, const Gradient& g2)
	{ return g1.extend == g2.extend && g1.stops == g2.stops; }
	friend inline bool operator!=(const Gradient& g1, const Gradient& g2)
	{ return !(g1 == g2); }

	static Gradient interpolate(const Gradient& g1, const Gradient& g2, PReal amt);
	static types::Color interpolate(const GradientStop& s1, const GradientStop& s2, PReal pos);
	static types::Color interpolate(const types::Color& v1, const types::Color& v2, PReal amt);

	static Gradient defaultGradient();

protected:
	PReal extendPos(PReal position) const;

	GradientStops stops;
	Extend extend;
};

} // namespace types

} // namespace panda

#endif // TYPES_GRADIENT_H
