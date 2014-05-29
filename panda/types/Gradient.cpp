#include <panda/types/Gradient.h>
#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

#include <QSet>
#include <qmath.h>

inline uint qHash(const PReal& key)
{
	return uint(key * 10000);
}

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
			stops.insert(i, qMakePair(position, color));
			return;
		}
	}

	// Or if the list is currently empty...
	stops.push_back(qMakePair(position, color));
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

inline bool compareStops(const QPair<PReal, Color> &p1, const QPair<PReal, Color> &p2)
{
	return p1.first < p2.first;
}

void Gradient::setStops(GradientStops stopsPoints)
{
	stops = stopsPoints;
	qStableSort(stops.begin(), stops.end(), compareStops);
}

Gradient::GradientStops Gradient::getStops() const
{
	if(stops.empty())
	{	// Same rule as Qt's: when empty, do instead a gradient from black to white
		GradientStops temp;
		temp.push_back(qMakePair(0, Color::black()));
		temp.push_back(qMakePair(1, Color::white()));
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
		return qBound<PReal>(0.0, position, 1.0);

	case EXTEND_REPEAT:
		return position - qFloor(position);

	case EXTEND_REFLECT:
		PReal p = position - qFloor(position);
		return ((qFloor(position) % 2) ? 1.0 - p : p);
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

	QSet<PReal> keys;
	for(const auto& stop : stops1)
	{
		PReal key = stop.first;
		if(!keys.contains(key))
		{
			keys.insert(key);
			Color color = interpolate(g1.get(key), g2.get(key), amt);
			grad.add(key, color);
		}
	}

	for(const auto& stop : stops2)
	{
		PReal key = stop.first;
		if(!keys.contains(key))
		{
			keys.insert(key);
			Color color = interpolate(g1.get(key), g2.get(key), amt);
			grad.add(key, color);
		}
	}

	return grad;
}

//***************************************************************//

template<> QString DataTrait<Gradient>::valueTypeName() { return "gradient"; }

template<>
void DataTrait<Gradient>::writeValue(QDomDocument& doc, QDomElement& elem, const Gradient& grad)
{
	auto colorTrait = DataTraitsList::getTraitOf<Color>();

	elem.setAttribute("extend", grad.getExtend());

	for(const auto& s : grad.getStops())
	{
		QDomElement stopNode = doc.createElement("Stop");
		elem.appendChild(stopNode);
		stopNode.setAttribute("pos", s.first);
		colorTrait->writeValue(doc, stopNode, &s.second);
	}
}

template<>
void DataTrait<Gradient>::readValue(QDomElement& elem, Gradient& grad)
{
	auto colorTrait = DataTraitsList::getTraitOf<Color>();

	grad.clear();
	grad.setExtend(elem.attribute("extend").toInt());

	QDomElement stopNode = elem.firstChildElement("Stop");
	while(!stopNode.isNull())
	{
#ifdef PANDA_DOUBLE
		PReal pos = stopNode.attribute("pos").toDouble();
#else
		PReal pos = stopNode.attribute("pos").toFloat();
#endif
		Color color;
		colorTrait->readValue(stopNode, &color);

		grad.add(pos, color);
		stopNode = stopNode.nextSiblingElement("Stop");
	}
}

template class Data< Gradient >;
template class Data< QVector<Gradient> >;

int gradientDataClass = RegisterData< Gradient >();
int gradientVectorDataClass = RegisterData< QVector<Gradient> >();

//*************************************************************************//

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
