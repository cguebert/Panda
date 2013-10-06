#ifndef ANIMATION_H
#define ANIMATION_H

#include <panda/DataAccessor.h>

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

} // namespace types

//***************************************************************//

template<class T>
class DataAccessorAnimation
{
public:
	typedef T container_type;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::reference reference;
	typedef typename container_type::const_reference const_reference;
	typedef typename container_type::Iterator Iterator;
	typedef typename container_type::keysList keysList;
	typedef typename container_type::valuesList valuesList;

protected:
	container_type& vref;

public:
	DataAccessorAnimation(container_type& container) : vref(container) {}
	~DataAccessorAnimation() {}

	const container_type& ref() const { return vref; }
	container_type& wref() { return vref; }

	value_type get(double at) { return vref.get(at); }
	reference getValue(double key) { return vref.getVakue(key); }
	reference getValueAtIndex(int index) { return vref.getValueAtIndex(index); }
	value_type getValueAtIndexConst(int index) const { vref.getValueAtIndexConst(index); }

	int size() const { return vref.size(); }
	void clear() { vref.clear(); }
	void add(double key, const_reference value) { vref.add(key, value); }
	void move(double keyFrom, double keyTo) { vref.move(keyFrom, keyTo); }
	void remove(double key) { vref.remove(key); }

	keysList getKeys() const { return vref.uniqueKeys(); }
	valuesList getValues() const { return vref.values(); }

	Iterator getIterator() const { return vref.getIterator(); }

	template<class U> void operator=(const U& v) { vref = v; }
};

template<class T>
class DataAccessor< types::Animation<T> > : public DataAccessorAnimation< types::Animation<T> >
{
public:
	typedef DataAccessorAnimation< types::Animation<T> > Inherit;
	typedef typename Inherit::container_type container_type;
	DataAccessor(container_type& c) : Inherit(c) {}
	template<class U> void operator=(const U& v) { Inherit::operator=(v); }
};

} // namespace panda

#endif // ANIMATION_H
