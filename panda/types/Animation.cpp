#include <panda/types/Animation.h>

#include <QColor>
#include <QPointF>

namespace panda
{

namespace types
{

template <class T>
typename Animation<T>::value_type Animation<T>::get(double at) const
{
	T defaultValue = T();
	if(map.isEmpty())
		return defaultValue;

	QList<double> keys = map.uniqueKeys();
	double lowBound = keys.front(), highBound = keys.back();

	if(at < lowBound)
		return map.value(lowBound);
	else if(at > highBound)
		return map.value(highBound);

	double keyA, keyB;
	keyA = keyB = keys.front();
	QListIterator<double> iter(keys);
	while(iter.hasNext())
	{
		double cur = iter.next();
		if(cur < at)
			keyA = keyB = cur;
		else
		{
			keyB = cur;
			break;
		}
	}

	double amt = (at-keyA) / (keyB-keyA);
	amt = interpolation.valueForProgress(amt);

	T valA, valB;
	valA = map.value(keyA);
	valB = map.value(keyB);

	return lerp(valA, valB, amt);
}

template <class T>
void Animation<T>::move(double keyFrom, double keyTo)
{
	T value = map.take(keyFrom);
	map.insert(keyTo, value);
}

template <class T>
typename Animation<T>::reference Animation<T>::getValueAtIndex(int index)
{
	typename QMap<double, T>::iterator iter;
	int i=0;
	for(iter=map.begin(); iter!=map.end(); ++iter, ++i)
		if(i == index)
			return iter.value();

	static T t;
	return t;	// Hack
}

template <class T>
typename Animation<T>::value_type Animation<T>::getValueAtIndexConst(int index) const
{
	typename QMap<double, T>::const_iterator iter;
	int i=0;
	for(iter=map.begin(); iter!=map.end(); ++iter, ++i)
		if(i == index)
			return iter.value();

	return T();
}

template<>
QColor lerp(const QColor& v1, const QColor& v2, double amt)
{
	qreal r1, r2, g1, g2, b1, b2, a1, a2;
	v1.getRgbF(&r1, &g1, &b1, &a1);
	v2.getRgbF(&r2, &g2, &b2, &a2);
	qreal r, g, b, a;
	r = lerp(r1, r2, amt);
	g = lerp(g1, g2, amt);
	b = lerp(b1, b2, amt);
	a = lerp(a1, a2, amt);
	QColor temp;
	temp.setRgbF(r, g, b, a);
	return temp;
}

template class Animation<double>;
template class Animation<QPointF>;
template class Animation<QColor>;

} // namespace types

} // namespace panda
