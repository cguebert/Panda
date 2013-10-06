#ifndef ANIMATION_H
#define ANIMATION_H

#include <QMap>
#include <QEasingCurve>
#include <QColor>

namespace panda
{

namespace types
{

template<class T> T lerp(const T& v1, const T& v2, double amt);

template<>
QColor lerp(const QColor& v1, const QColor& v2, double amt);

template<class T> T lerp(const T& v1, const T& v2, double amt)
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
	typedef QMapIterator<double, T> Iterator;
	typedef QList<double> keysList;
	typedef QList<value_type> valuesList;

	Animation() {}
	virtual ~Animation() {}

	value_type get(double at) const;
	reference getValue(double key) { return map[key]; }
	reference getValueAtIndex(int index);
	value_type getValueAtIndexConst(int index) const;

	int size() const { return map.size(); }
	void clear() { map.clear(); }
	void add(double key, const_reference value) { map.insert(key, value); }
	void move(double keyFrom, double keyTo);
	void remove(double key) { map.remove(key); }

	virtual keysList getKeys() const { return map.uniqueKeys(); }
	valuesList getValues() const { return map.values(); }

	Iterator getIterator() const { return Iterator(map); }

	QEasingCurve interpolation;

protected:
	QMap<double, T> map;
};

typedef Animation<QColor> Gradient;

} // namespace types

} // namespace panda

#endif // ANIMATION_H
