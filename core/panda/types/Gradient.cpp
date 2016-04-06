#include <panda/types/Gradient.h>
#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/data/DataFactory.h>

#include <cmath>

namespace panda
{

namespace types
{

using types::Color;

Gradient::Gradient()
	: m_extend(Extend::Pad) 
{
	m_stops.emplace_back(0.f, Color::black());
	m_stops.emplace_back(1.f, Color::white());
}

void Gradient::add(float position, Color color)
{
	if(position < 0 || position > 1)
		return; // Ignore invalid insertions

	// Insert already at the right place
	auto it = std::lower_bound(m_stops.begin(), m_stops.end(), position, [](const GradientStop& stop, float pos) {
		return stop.first < pos;
	});
	m_stops.emplace(it, position, color);
}

Color Gradient::get(float position) const
{
	float pos = extendPos(position);

	int nb = m_stops.size();
	if(!nb)	// Same rule as Qt's: when empty, do instead a gradient from black to white
		return interpolate(Color::black(), Color::white(), pos);
	else if(nb == 1)
		return m_stops.front().second;
	else if(pos <= m_stops.front().first)
		return m_stops.front().second;
	else if(pos >= m_stops.back().first)
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

void Gradient::setExtendInt(int method)
{
	int val = helper::bound(0, method, static_cast<int>(Extend::Reflect));
	setExtend(static_cast<Extend>(val));
}

void Gradient::setStops(GradientStops stopsPoints)
{
	m_stops = stopsPoints;
	std::stable_sort(m_stops.begin(), m_stops.end(), [](const GradientStop& lhs, const GradientStop& rhs){
		return lhs.first < rhs.first;
	});
}

float Gradient::extendPos(float position) const
{
	switch(m_extend)
	{
	default:
	case Extend::Pad:
		return std::max<float>(0, std::min<float>(position, 1));

	case Extend::Repeat:
		return position - std::floor(position);

	case Extend::Reflect:
		float p = position - std::floor(position);
		return ((static_cast<int>(std::floor(position)) % 2) ? 1 - p : p);
	}
}

Gradient::KeysList Gradient::keys() const
{
	KeysList tmp;
	for(const auto& stop : m_stops)
		tmp.push_back(stop.first);
	return tmp;
}

Gradient::ColorsList Gradient::colors() const
{
	ColorsList tmp;
	for(const auto& stop : m_stops)
		tmp.push_back(stop.second);
	return tmp;
}

Color Gradient::interpolate(const GradientStop& s1, const GradientStop& s2, float pos)
{
	float amt = (pos - s1.first) / (s2.first - s1.first);
	return interpolate(s1.second, s2.second, amt);
}

Color Gradient::interpolate(const Color& v1, const Color& v2, float amt)
{
	return v1 + (v2 - v1) * amt;
}

Gradient Gradient::interpolate(const Gradient& g1, const Gradient& g2, float amt)
{
	Gradient grad;
	grad.clear();
	const auto& stops1 = g1.stops();
	const auto& stops2 = g2.stops();

	// Merging the keys of the 2 gradients, sorting them and making sure they are unique
	std::vector<float> keys;
	for (const auto& stop : stops1)
		keys.push_back(stop.first);
	for (const auto& stop : stops2)
		keys.push_back(stop.first);
	std::sort(keys.begin(), keys.end());
	auto last = std::unique(keys.begin(), keys.end());
	keys.erase(last, keys.end());

	// Creating a gradient with stops at each of the original ones
	for (const auto& key : keys)
	{
		Color color = interpolate(g1.get(key), g2.get(key), amt);
		grad.add(key, color);
	}

	return grad;
}

//****************************************************************************//

template<> PANDA_CORE_API std::string DataTrait<Gradient>::valueTypeName() { return "gradient"; }

template<> PANDA_CORE_API unsigned int DataTrait<Gradient>::typeColor() { return 0x83A1C7; }

template<>
PANDA_CORE_API void DataTrait<Gradient>::writeValue(XmlElement& elem, const Gradient& grad)
{
	auto colorTrait = DataTraitsList::getTraitOf<Color>();

	elem.setAttribute("extend", static_cast<int>(grad.extend()));

	for(const auto& s : grad.stops())
	{
		auto stopNode = elem.addChild("Stop");
		stopNode.setAttribute("pos", s.first);
		colorTrait->writeValue(stopNode, &s.second);
	}
}

template<>
PANDA_CORE_API void DataTrait<Gradient>::readValue(XmlElement& elem, Gradient& grad)
{
	auto colorTrait = DataTraitsList::getTraitOf<Color>();

	grad.clear();
	grad.setExtendInt(elem.attribute("extend").toInt());

	auto stopNode = elem.firstChild("Stop");
	while(stopNode)
	{
		float pos = stopNode.attribute("pos").toFloat();
		Color color;
		colorTrait->readValue(stopNode, &color);

		grad.add(pos, color);
		stopNode = stopNode.nextSibling("Stop");
	}
}

template class PANDA_CORE_API Data< Gradient >;
template class PANDA_CORE_API Data< std::vector<Gradient> >;

int gradientDataClass = RegisterData< Gradient >();
int gradientVectorDataClass = RegisterData< std::vector<Gradient> >();

//****************************************************************************//

template<>
Gradient interpolate(const Gradient& g1, const Gradient& g2, float amt)
{ return Gradient::interpolate(g1, g2, amt); }

template class PANDA_CORE_API Animation<Gradient>;
template class PANDA_CORE_API Data< Animation<Gradient> >;
int gradientAnimationDataClass = RegisterData< Animation<Gradient> >();

} // namespace types

} // namespace panda
