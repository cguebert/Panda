#ifndef TYPES_GRADIENT_H
#define TYPES_GRADIENT_H

#include <panda/core.h>
#include <panda/types/Color.h>

#include <vector>

namespace panda
{

namespace types
{

class PANDA_CORE_API Gradient
{
public:
	typedef std::pair<float, types::Color> GradientStop;
	typedef std::vector<GradientStop> GradientStops;

	enum Extend { EXTEND_PAD = 0, EXTEND_REPEAT = 1, EXTEND_REFLECT = 2 };

	Gradient();

	void clear();

	void add(float position, types::Color color);
	types::Color get(float position) const;

	void setExtend(int method);
	int getExtend() const;

	void setStops(GradientStops stopsPoints);
	GradientStops getStops() const;
	GradientStops getStopsForEdit() const; // this one doesn't lie

	friend inline bool operator==(const Gradient& g1, const Gradient& g2)
	{ return g1.extend == g2.extend && g1.stops == g2.stops; }
	friend inline bool operator!=(const Gradient& g1, const Gradient& g2)
	{ return !(g1 == g2); }

	static Gradient interpolate(const Gradient& g1, const Gradient& g2, float amt);
	static types::Color interpolate(const GradientStop& s1, const GradientStop& s2, float pos);
	static types::Color interpolate(const types::Color& v1, const types::Color& v2, float amt);

	static Gradient defaultGradient();

protected:
	float extendPos(float position) const;

	GradientStops stops;
	Extend extend;
};

} // namespace types

} // namespace panda

#endif // TYPES_GRADIENT_H
