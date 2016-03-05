#ifndef ANIMATION_INL
#define ANIMATION_INL

#include <panda/types/Animation.h>
#include <panda/helper/algorithm.h>

namespace panda
{

namespace types
{

template <class T>
void Animation<T>::add(float position, value_type value)
{
	// Insert already at the right place
	auto it = std::lower_bound(m_stops.begin(), m_stops.end(), position, [](const AnimationStop& stop, float pos) {
		return stop.first < pos;
	});
	m_stops.emplace(it, position, value);
}

template <class T>
typename Animation<T>::value_type Animation<T>::get(float position) const
{
	int nb = m_stops.size();
	if(!nb)
		return value_type();
	else if(nb == 1)
		return m_stops.front().second;

	float pMin = m_stops.front().first;
	float pMax = m_stops.back().first;
	float pos = 0.5;
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
void Animation<T>::setInterpolationInt(int method)
{
	int val = helper::bound(0, method, static_cast<int>(helper::EasingFunctions::Type::OutInBack));
	setInterpolation(static_cast<helper::EasingFunctions::Type>(val));
}

template <class T>
void Animation<T>::setExtendInt(int method)
{
	int val = helper::bound(0, method, static_cast<int>(Extend::Reflect));
	setExtend(static_cast<Extend>(val));
}

template <class T>
void Animation<T>::setStops(AnimationStops stopsPoints)
{
	m_stops = stopsPoints;
	std::stable_sort(m_stops.begin(), m_stops.end(), [](const AnimationStop& lhs, const AnimationStop& rhs){
		return lhs.first < rhs.first;
	});
}

template <class T>
typename Animation<T>::KeysList Animation<T>::keys() const
{
	KeysList tmp;
	for(const auto& stop : m_stops)
		tmp.push_back(stop.first);
	return tmp;
}

template <class T>
typename Animation<T>::ValuesList Animation<T>::values() const
{
	ValuesList tmp;
	for(const auto& stop : m_stops)
		tmp.push_back(stop.second);
	return tmp;
}

template <class T>
float Animation<T>::extendPos(float position, float pMin, float pMax) const
{
	switch(m_extend)
	{
	default:
	case Extend::Pad:
		return std::max(pMin, std::min(position, pMax));

	case Extend::Repeat:
	{
		float w = pMax - pMin;
		float p = (position - pMin) / w;
		p = p - std::floor(p);
		return pMin + p * w;
	}

	case Extend::Reflect:
	{
		float w = pMax - pMin;
		float p = (position - pMin) / w;
		float t = position - std::floor(position);
		p = ((static_cast<int>(std::floor(p)) % 2) ? 1.f - t : t);
		return pMin + p * w;
	}
	}
}

template <class T>
typename Animation<T>::value_type Animation<T>::interpolate(const AnimationStop& s1, const AnimationStop& s2, float pos) const
{
	float amt = (pos - s1.first) / (s2.first - s1.first);
	amt = m_interpolation.valueForProgress(amt);
	return types::interpolate(s1.second, s2.second, amt);
}

} // namespace types

} // namespace panda

#endif // ANIMATION_INL
