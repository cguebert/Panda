#ifndef ANIMATION_INL
#define ANIMATION_INL

#include <panda/types/Animation.h>
#include <algorithm>

namespace panda
{

namespace types
{

template <class T>
int Animation<T>::size() const
{
	return m_stops.size();
}

template <class T>
void Animation<T>::clear()
{
	m_stops.clear();
}

template <class T>
void Animation<T>::add(PReal position, value_type value)
{
	// Insert already at the right place
	for(int i=0, nb=m_stops.size(); i<nb; ++i)
	{
		if(position < m_stops[i].first)
		{
			m_stops.insert(m_stops.begin() + i, std::make_pair(position, value));
			return;
		}
	}

	// Or if the list is currently empty...
	m_stops.push_back(std::make_pair(position, value));
}

template <class T>
typename Animation<T>::value_type Animation<T>::get(PReal position) const
{
	int nb = m_stops.size();
	if(!nb)
		return value_type();
	else if(nb == 1)
		return m_stops.front().second;

	PReal pMin = m_stops.front().first;
	PReal pMax = m_stops.back().first;
	PReal pos = 0.5;
	if(pMax - pMin > 1e-10) // If the interval is too small, consider pos to be in the middle
		pos = extendPos(position, pMin, pMax);

	if(pos <= pMin)
		return m_stops.front().second;
	else if(pos >= pMax)
		return m_stops.back().second;
	else if(nb == 2)
		return interpolate(m_stops.front(), m_stops.back(), pos);
	else
	{
		int i;
		for(i=0; i+2<nb; ++i)
		{
			if(m_stops[i+1].first > pos)
				break;
		}

		return interpolate(m_stops[i], m_stops[i+1], pos);
	}
}

template <class T>
typename Animation<T>::reference Animation<T>::getAtIndex(int index)
{
	if(index < 0 || index >= static_cast<int>(m_stops.size()))
	{
		static value_type tmp = value_type();
		return tmp;
	}
	return m_stops[index].second;
}

template <class T>
typename Animation<T>::const_reference Animation<T>::getAtIndex(int index) const
{
	if(index < 0 || index >= static_cast<int>(m_stops.size()))
	{
		static value_type tmp = value_type();
		return tmp;
	}
	return m_stops[index].second;
}

template <class T>
void Animation<T>::setInterpolation(int method)
{
	m_interpolation.setType(static_cast<helper::EasingFunctions::Type>(method));
}

template <class T>
int Animation<T>::getInterpolation() const
{
	return static_cast<int>(m_interpolation.type());
}

template <class T>
void Animation<T>::setExtend(int method)
{
	m_extend = static_cast<Extend>(method);
}

template <class T>
int Animation<T>::getExtend() const
{
	return static_cast<int>(m_extend);
}

template <class T>
inline bool compareStops(const std::pair<PReal, T> &p1, const std::pair<PReal, T> &p2)
{
	return p1.first < p2.first;
}

template <class T>
void Animation<T>::setStops(typename Animation<T>::AnimationStops stopsPoints)
{
	m_stops = stopsPoints;
	std::stable_sort(m_stops.begin(), m_stops.end(), compareStops<T>);
}

template <class T>
typename Animation<T>::AnimationStops Animation<T>::getStops() const
{
	return m_stops;
}

template <class T>
typename Animation<T>::KeysList Animation<T>::getKeys() const
{
	KeysList tmp;
	for(const auto& stop : m_stops)
		tmp.push_back(stop.first);
	return tmp;
}

template <class T>
typename Animation<T>::ValuesList Animation<T>::getValues() const
{
	ValuesList tmp;
	for(const auto& stop : m_stops)
		tmp.push_back(stop.second);
	return tmp;
}

template <class T>
PReal Animation<T>::extendPos(PReal position, PReal pMin, PReal pMax) const
{
	switch(m_extend)
	{
	default:
	case Extend::Pad:
		return std::max(pMin, std::min(position, pMax));

	case Extend::Repeat:
	{
		PReal w = pMax - pMin;
		PReal p = (position - pMin) / w;
		p = p - std::floor(p);
		return pMin + p * w;
	}

	case Extend::Reflect:
	{
		PReal w = pMax - pMin;
		PReal p = (position - pMin) / w;
		PReal t = position - std::floor(position);
		p = ((static_cast<int>(std::floor(p)) % 2) ? 1.f - t : t);
		return pMin + p * w;
	}
	}
}

template <class T>
typename Animation<T>::value_type Animation<T>::interpolate(const AnimationStop& s1, const AnimationStop& s2, PReal pos) const
{
	PReal amt = (pos - s1.first) / (s2.first - s1.first);
	amt = m_interpolation.valueForProgress(amt);
	return types::interpolate(s1.second, s2.second, amt);
}

} // namespace types

} // namespace panda

#endif // ANIMATION_INL
