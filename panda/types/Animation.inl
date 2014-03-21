#ifndef ANIMATION_INL
#define ANIMATION_INL

#include <panda/types/Animation.h>

#include <QColor>
#include <QPointF>
#include <QEasingCurve>

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
void Animation<T>::add(PReal position, value_type value)
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
typename Animation<T>::value_type Animation<T>::get(PReal position) const
{
	int nb = stops.size();
	if(!nb)
		return value_type();
	else if(nb == 1)
		return stops.front().second;

	PReal pMin = stops.front().first;
	PReal pMax = stops.back().first;
	PReal pos = 0.5;
	if(pMax - pMin > 1e-10) // If the interval is too small, consider pos to be in the middle
		pos = extendPos(position, pMin, pMax);

	if(pos <= pMin)
		return stops.front().second;
	else if(pos >= pMax)
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
typename Animation<T>::const_reference Animation<T>::getAtIndex(int index) const
{
	if(index < 0 || index >= stops.size())
	{
		static value_type tmp = value_type();
		return tmp;
	}
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
inline bool compareStops(const QPair<PReal, T> &p1, const QPair<PReal, T> &p2)
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
PReal Animation<T>::extendPos(PReal position, PReal pMin, PReal pMax) const
{
	switch(extend)
	{
	default:
	case EXTEND_PAD:
		return qBound(pMin, position, pMax);

	case EXTEND_REPEAT:
	{
		PReal w = pMax - pMin;
		PReal p = (position - pMin) / w;
		p = p - qFloor(p);
		return pMin + p * w;
	}

	case EXTEND_REFLECT:
	{
		PReal w = pMax - pMin;
		PReal p = (position - pMin) / w;
		PReal t = position - qFloor(position);
		p = ((qFloor(p) % 2) ? 1.0 - t : t);
		return pMin + p * w;
	}
	}
}

template <class T>
typename Animation<T>::value_type Animation<T>::interpolate(const AnimationStop& s1, const AnimationStop& s2, PReal pos) const
{
	PReal amt = (pos - s1.first) / (s2.first - s1.first);
	QEasingCurve ec(static_cast<QEasingCurve::Type>(interpolation));
	amt = ec.valueForProgress(amt);
	return types::interpolate(s1.second, s2.second, amt);
}

} // namespace types

} // namespace panda

#endif // ANIMATION_INL
