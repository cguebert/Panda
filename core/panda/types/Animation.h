#ifndef TYPES_ANIMATION_H
#define TYPES_ANIMATION_H

#include <panda/helper/EasingFunctions.h>

namespace panda
{

namespace types
{

template<class T> T interpolate(const T& v1, const T& v2, PReal amt);

template<class T> T interpolate(const T& v1, const T& v2, PReal amt)
{
	return v1 + (v2 - v1) * amt;
}

template <class T>
class PANDA_CORE_API Animation
{
public:
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::pair<PReal, value_type> AnimationStop;
	typedef std::vector<AnimationStop> AnimationStops;
	typedef std::vector<PReal> KeysList;
	typedef std::vector<value_type> ValuesList;

	enum class Extend { Pad, Repeat, Reflect };

	int size() const;
	void clear();

	void add(PReal position, value_type value);
	value_type get(PReal position) const;

	reference getAtIndex(int index);
	const_reference getAtIndex(int index) const;

	void setInterpolation(int method);
	int getInterpolation() const;

	void setExtend(int method);
	int getExtend() const;

	void setStops(AnimationStops stopsPoints);
	AnimationStops getStops() const;
	KeysList getKeys() const;
	ValuesList getValues() const;

	inline bool operator==(const Animation& rhs)
	{
		return m_interpolation.type() == rhs.m_interpolation.type()
				&& m_extend == rhs.m_extend
				&& m_stops == rhs.m_stops;
	}

	inline bool operator!=(const Animation& rhs)
	{
		return !(*this == rhs);
	}

protected:
	PReal extendPos(PReal position, PReal pMin, PReal pMax) const;
	value_type interpolate(const AnimationStop& s1, const AnimationStop& s2, PReal pos) const;

	helper::EasingFunctions m_interpolation;
	Extend m_extend = Extend::Pad;
	AnimationStops m_stops;
};

} // namespace types

} // namespace panda

#endif // TYPES_ANIMATION_H
