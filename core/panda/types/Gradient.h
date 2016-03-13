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
	typedef std::vector<float> KeysList;
	typedef std::vector<types::Color> ColorsList;

	enum class Extend { Pad, Repeat, Reflect };

	Gradient();

	int size() const;
	void clear();

	void add(float position, types::Color color);
	types::Color get(float position) const;

	types::Color& colorAtIndex(int index); // Not bounds checked
	const types::Color& colorAtIndex(int index) const; // Not bounds checked

	void removeAtIndex(int index); // Not bounds checked

	void setExtend(Extend method);
	void setExtendInt(int method);
	Extend extend() const;

	void setStops(GradientStops stopsPoints);
	const GradientStops& stops() const;

	KeysList keys() const;
	ColorsList colors() const;

	friend bool operator==(const Gradient& g1, const Gradient& g2);
	friend bool operator!=(const Gradient& g1, const Gradient& g2);

	static Gradient interpolate(const Gradient& g1, const Gradient& g2, float amt);
	static types::Color interpolate(const GradientStop& s1, const GradientStop& s2, float pos);
	static types::Color interpolate(const types::Color& v1, const types::Color& v2, float amt);

protected:
	float extendPos(float position) const;

	GradientStops m_stops;
	Extend m_extend;
};

inline Gradient interpolate(const Gradient& g1, const Gradient& g2, float amt)
{ return Gradient::interpolate(g1, g2, amt); }

inline int Gradient::size() const
{ return m_stops.size(); }

inline void Gradient::clear()
{ m_stops.clear(); }

inline types::Color& Gradient::colorAtIndex(int index) 
{ return m_stops[index].second; }
	
inline const types::Color& Gradient::colorAtIndex(int index) const
{ return m_stops[index].second; }

inline void Gradient::removeAtIndex(int index)
{ m_stops.erase(m_stops.begin() + index); }

inline void Gradient::setExtend(Extend method)
{ m_extend = method; }

inline Gradient::Extend Gradient::extend() const
{ return m_extend; }

inline const Gradient::GradientStops& Gradient::stops() const
{ return m_stops; }

inline bool operator==(const Gradient& g1, const Gradient& g2)
{ return g1.extend() == g2.extend() && g1.stops() == g2.stops(); }

inline bool operator!=(const Gradient& g1, const Gradient& g2)
{ return !(g1 == g2); }

} // namespace types

} // namespace panda

#endif // TYPES_GRADIENT_H
