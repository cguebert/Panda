#ifndef ANIMATION_H
#define ANIMATION_H

#include <QVector>
#include <QColor>

namespace panda
{

namespace types
{

template<class T> T interpolate(const T& v1, const T& v2, double amt);

template<>
QColor interpolate(const QColor& v1, const QColor& v2, double amt);

template<class T> T interpolate(const T& v1, const T& v2, double amt)
{
	return v1 * (1.0-amt) + v2 * amt;
}

template <class T>
class Animation
{
public:
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef QPair<double, value_type> AnimationStop;
	typedef QVector<AnimationStop> AnimationStops;
	typedef QVector<double> KeysList;
	typedef QVector<value_type> ValuesList;

	enum Extend { EXTEND_PAD = 0, EXTEND_REPEAT = 1, EXTEND_REFLECT = 2 };

	Animation();

	int size() const;
	void clear();

	void add(double position, value_type value);
	value_type get(double position) const;

	reference getAtIndex(int index);
	value_type getAtIndex(int index) const;

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
		return interpolation == rhs.interpolation
				&& extend == rhs.extend
				&& stops == rhs.stops;
	}

	inline bool operator!=(const Animation& rhs)
	{
		return !(*this == rhs);
	}

protected:
	double extendPos(double position, double pMin, double pMax) const;
	value_type interpolate(const AnimationStop& s1, const AnimationStop& s2, double pos) const;

	int interpolation;
	Extend extend;
	AnimationStops stops;
};

} // namespace types

} // namespace panda

#endif // ANIMATION_H
