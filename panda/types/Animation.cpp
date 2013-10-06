#include <panda/types/Animation.h>

#include <QColor>
#include <QPointF>

#include <qmath.h>

namespace panda
{

namespace types
{

template <class T>
Animation<T>::Animation()
	: interpolation(QEasingCurve::Linear)
	, extend(EXTEND_PAD)
{}

template <class T>
int Animation<T>::size() const
{
	return stops.size();
}

template <class T>
void Animation<T>::clear()
{
	stops.clear();
}

template <class T>
void Animation<T>::add(qreal position, value_type value)
{
	// Insert already at the right place
	for(int i=0, nb=stops.size(); i<nb; ++i)
	{
		if(position < stops[i].first)
		{
			stops.insert(i, qMakePair(position, value));
			return;
		}
	}

	// Or if the list is currently empty...
	stops.push_back(qMakePair(position, value));
}

template <class T>
typename Animation<T>::value_type Animation<T>::get(qreal position) const
{
	qreal pos = extendPos(position);

	int nb = stops.size();
	if(!nb)
		return value_type();
	else if(nb == 1)
		return stops.front().second;
	else if(pos <= stops.front().first)
		return stops.front().second;
	else if(pos >= stops.back().first)
		return stops.back().second;
	else if(nb == 2)
		return interpolate(stops.front(), stops.back(), pos);
	else
	{
		int i;
		for(i=0; i+2<nb; ++i)
		{
			if(stops[i+1].first > pos)
				break;
		}

		return interpolate(stops[i], stops[i+1], pos);
	}
}

template <class T>
typename Animation<T>::reference Animation<T>::getAtIndex(int index)
{
	if(index < 0 || index >= stops.size())
	{
		static value_type tmp = value_type();
		return tmp;
	}
	return stops[index].second;
}

template <class T>
typename Animation<T>::value_type Animation<T>::getAtIndex(int index) const
{
	if(index < 0 || index >= stops.size())
		return value_type();
	return stops[index].second;
}

template <class T>
void Animation<T>::setInterpolation(int method)
{
	interpolation = method;
}

template <class T>
int Animation<T>::getInterpolation() const
{
	return interpolation;
}

template <class T>
void Animation<T>::setExtend(int method)
{
	extend = static_cast<Extend>(method);
}

template <class T>
int Animation<T>::getExtend() const
{
	return extend;
}

template <class T>
inline bool compareStops(const QPair<qreal, T> &p1, const QPair<qreal, T> &p2)
{
	return p1.first < p2.first;
}

template <class T>
void Animation<T>::setStops(typename Animation<T>::AnimationStops stopsPoints)
{
	stops = stopsPoints;
	qStableSort(stops.begin(), stops.end(), compareStops<T>);
}

template <class T>
typename Animation<T>::AnimationStops Animation<T>::getStops() const
{
	return stops;
}

template <class T>
typename Animation<T>::KeysList Animation<T>::getKeys() const
{
	KeysList tmp;
	for(auto stop : stops)
		tmp.push_back(stop.first);
	return tmp;
}

template <class T>
typename Animation<T>::ValuesList Animation<T>::getValues() const
{
	ValuesList tmp;
	for(auto stop : stops)
		tmp.push_back(stop.second);
	return tmp;
}

template <class T>
qreal Animation<T>::extendPos(qreal position) const
{
	switch(extend)
	{
	default:
	case EXTEND_PAD:
		return qBound(0.0, position, 1.0);

	case EXTEND_REPEAT:
		return position - qFloor(position);

	case EXTEND_REFLECT:
		qreal p = qBound(0.0, position, 1.0);
		return (qFloor(position) % 2 ? 1.0 - p : p);
	}
}

template <class T>
typename Animation<T>::value_type Animation<T>::interpolate(const AnimationStop& s1, const AnimationStop& s2, qreal pos) const
{
	qreal amt = (pos - s1.first) / (s2.first - s1.first);
	QEasingCurve ec(static_cast<QEasingCurve::Type>(interpolation));
	amt = ec.valueForProgress(amt);
	return types::interpolate(s1.second, s2.second, amt);
}

//***************************************************************//

template<>
QColor interpolate(const QColor& v1, const QColor& v2, double amt)
{
	qreal r1, r2, g1, g2, b1, b2, a1, a2;
	v1.getRgbF(&r1, &g1, &b1, &a1);
	v2.getRgbF(&r2, &g2, &b2, &a2);
	qreal r, g, b, a;
	r = interpolate(r1, r2, amt);
	g = interpolate(g1, g2, amt);
	b = interpolate(b1, b2, amt);
	a = interpolate(a1, a2, amt);
	QColor temp;
	temp.setRgbF(r, g, b, a);
	return temp;
}

template class Animation<double>;
template class Animation<QPointF>;
template class Animation<QColor>;

} // namespace types

} // namespace panda
