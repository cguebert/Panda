#include <panda/types/Gradient.h>
#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/DataFactory.h>
#include <panda/Data.h>

#include <cmath>

namespace panda
{

namespace types
{

using types::Color;

Gradient::Gradient()
	: extend(EXTEND_PAD)
{ }

void Gradient::clear()
{
	stops.clear();
}

void Gradient::add(PReal position, Color color)
{
	if(position < 0 || position > 1)
		return; // Ignore invalid insertions

	// Insert already at the right place
	for(int i=0, nb=stops.size(); i<nb; ++i)
	{
		if(position < stops[i].first)
		{
			stops.insert(stops.begin() + i, std::make_pair(position, color));
			return;
		}
	}

	// Or if the list is currently empty...
	stops.emplace_back(position, color);
}

Color Gradient::get(PReal position) const
{
	PReal pos = extendPos(position);

	int nb = stops.size();
	if(!nb)	// Same rule as Qt's: when empty, do instead a gradient from black to white
		return interpolate(Color::black(), Color::white(), pos);
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

void Gradient::setExtend(int method)
{
	extend = static_cast<Extend>(method);
}

int Gradient::getExtend() const
{
	return extend;
}

inline bool compareStops(const std::pair<PReal, Color> &p1, const std::pair<PReal, Color> &p2)
{
	return p1.first < p2.first;
}

void Gradient::setStops(GradientStops stopsPoints)
{
	stops = stopsPoints;
	std::stable_sort(stops.begin(), stops.end(), compareStops);
}

Gradient::GradientStops Gradient::getStops() const
{
	if(stops.empty())
	{	// Same rule as Qt's: when empty, do instead a gradient from black to white
		GradientStops temp;
		temp.emplace_back(0.f, Color::black());
		temp.emplace_back(1.f, Color::white());
		return temp;
	}

	return stops;
}

Gradient::GradientStops Gradient::getStopsForEdit() const
{
	return stops;
}

PReal Gradient::extendPos(PReal position) const
{
	switch(extend)
	{
	default:
	case EXTEND_PAD:
		return std::max<PReal>(0, std::min<PReal>(position, 1));

	case EXTEND_REPEAT:
		return position - std::floor(position);

	case EXTEND_REFLECT:
		PReal p = position - std::floor(position);
		return ((static_cast<int>(std::floor(position)) % 2) ? 1 - p : p);
	}
}

Color Gradient::interpolate(const GradientStop& s1, const GradientStop& s2, PReal pos)
{
	PReal amt = (pos - s1.first) / (s2.first - s1.first);
	return interpolate(s1.second, s2.second, amt);
}

Color Gradient::interpolate(const Color& v1, const Color& v2, PReal amt)
{
	return v1 + (v2 - v1) * amt;
}

Gradient Gradient::interpolate(const Gradient& g1, const Gradient& g2, PReal amt)
{
	Gradient grad;
	const auto& stops1 = g1.getStops();
	const auto& stops2 = g2.getStops();

	// Merging the keys of the 2 gradients, sorting them and making sure they are unique
	std::vector<PReal> keys;
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

Gradient Gradient::defaultGradient()
{
	Gradient grad;
	grad.add(0, Color::black());
	grad.add(1, Color::white());
	return grad;
}

//****************************************************************************//

template<> PANDA_CORE_API std::string DataTrait<Gradient>::valueTypeName() { return "gradient"; }

template<>
PANDA_CORE_API void DataTrait<Gradient>::writeValue(XmlElement& elem, const Gradient& grad)
{
	auto colorTrait = DataTraitsList::getTraitOf<Color>();

	elem.setAttribute("extend", grad.getExtend());

	for(const auto& s : grad.getStops())
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
	grad.setExtend(elem.attribute("extend").toInt());

	auto stopNode = elem.firstChild("Stop");
	while(stopNode)
	{
#ifdef PANDA_DOUBLE
		PReal pos = stopNode.attribute("pos").toDouble();
#else
		PReal pos = stopNode.attribute("pos").toFloat();
#endif
		Color color;
		colorTrait->readValue(stopNode, &color);

		grad.add(pos, color);
		stopNode = stopNode.nextSibling("Stop");
	}
}

template class Data< Gradient >;
template class Data< std::vector<Gradient> >;

int gradientDataClass = RegisterData< Gradient >();
int gradientVectorDataClass = RegisterData< std::vector<Gradient> >();

//****************************************************************************//

template<>
Gradient interpolate(const Gradient& g1, const Gradient& g2, PReal amt)
{
	return Gradient::interpolate(g1, g2, amt);
}

template class Animation<Gradient>;
template class Data< Animation<Gradient> >;
int gradientAnimationDataClass = RegisterData< Animation<Gradient> >();

} // namespace types

} // namespace panda
