#include <panda/types/Gradient.h>
#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>
#include <panda/Data.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

#include <ui/SimpleDataWidget.h>

#include <QSet>
#include <qmath.h>

inline uint qHash(const double& key)
{
	return uint(key * 10000);
}

namespace panda
{

namespace types
{

Gradient::Gradient()
	: extend(EXTEND_PAD)
{ }

void Gradient::clear()
{
	stops.clear();
}

void Gradient::add(double position, QColor color)
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

QColor Gradient::get(double position) const
{
	double pos = extendPos(position);

	int nb = stops.size();
	if(!nb)
		return QColor();
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

inline bool compareStops(const QPair<double, QColor> &p1, const QPair<double, QColor> &p2)
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
	return stops;
}

double Gradient::extendPos(double position) const
{
	switch(extend)
	{
	default:
	case EXTEND_PAD:
		return qBound(0.0, position, 1.0);

	case EXTEND_REPEAT:
		return position - qFloor(position);

	case EXTEND_REFLECT:
		double p = position - qFloor(position);
		return ((qFloor(position) % 2) ? 1.0 - p : p);
	}
}

double Gradient::interpolate(double v1, double v2, double amt)
{
	return v1 * (1.0-amt) + v2 * amt;
}

QColor Gradient::interpolate(const GradientStop& s1, const GradientStop& s2, double pos)
{
	double amt = (pos - s1.first) / (s2.first - s1.first);
	return interpolate(s1.second, s2.second, amt);
}

QColor Gradient::interpolate(const QColor& v1, const QColor& v2, double amt)
{
	double r1, r2, g1, g2, b1, b2, a1, a2;
	v1.getRgbF(&r1, &g1, &b1, &a1);
	v2.getRgbF(&r2, &g2, &b2, &a2);
	double r, g, b, a;
	r = interpolate(r1, r2, amt);
	g = interpolate(g1, g2, amt);
	b = interpolate(b1, b2, amt);
	a = interpolate(a1, a2, amt);
	QColor temp;
	temp.setRgbF(r, g, b, a);
	return temp;
}

Gradient Gradient::interpolate(const Gradient& g1, const Gradient& g2, double amt)
{
	Gradient grad;
	const auto& stops1 = g1.getStops();
	const auto& stops2 = g2.getStops();

	QSet<double> keys;
	for(const auto& stop : stops1)
	{
		double key = stop.first;
		if(!keys.contains(key))
		{
			keys.insert(key);
			QColor color = interpolate(g1.get(key), g2.get(key), amt);
			grad.add(key, color);
		}
	}

	for(const auto& stop : stops2)
	{
		double key = stop.first;
		if(!keys.contains(key))
		{
			keys.insert(key);
			QColor color = interpolate(g1.get(key), g2.get(key), amt);
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
	elem.setAttribute("extend", grad.getExtend());

	for(const auto& s : grad.getStops())
	{
		QDomElement stopNode = doc.createElement("Stop");
		elem.appendChild(stopNode);
		stopNode.setAttribute("pos", s.first);
		DataTrait<QColor>::writeValue(doc, stopNode, s.second);
	}
}

template<>
void DataTrait<Gradient>::readValue(QDomElement& elem, Gradient& grad)
{
	grad.clear();
	grad.setExtend(elem.attribute("extend").toInt());

	QDomElement stopNode = elem.firstChildElement("Stop");
	while(!stopNode.isNull())
	{
		double pos = stopNode.attribute("pos").toDouble();
		QColor color;
		DataTrait<QColor>::readValue(stopNode, color);

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
Gradient interpolate(const Gradient& g1, const Gradient& g2, double amt)
{
	return Gradient::interpolate(g1, g2, amt);
}

template class Animation<Gradient>;
template class Data< Animation<Gradient> >;
int gradientAnimationDataClass = RegisterData< Animation<Gradient> >();

//*************************************************************************//

const char* Gradient::extendModes[] = { "Pad", "Repeat", "Reflect"};
Creator<DataWidgetFactory, SimpleDataWidget<int, EnumDataWidget<3, Gradient::extendModes> > > DWClass_enum_gradient_extend("enum_GradientExtend",true);

} // namespace types

} // namespace panda
