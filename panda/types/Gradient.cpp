#include <panda/types/Gradient.h>
#include <panda/Data.h>
#include <panda/DataFactory.h>

#include <qmath.h>

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

void Gradient::add(qreal position, QColor color)
{
	// Insert already at the right place
	for(int i=0, nb=stops.size(); i<nb; ++i)
	{
		if(position < stops[i].first)
		{
			stops.insert(i, qMakePair(position, color));
			return;
		}
	}

	// If the list is currently empty
	stops.push_back(qMakePair(position, color));
}

QColor Gradient::get(qreal position) const
{
	qreal pos = extendPos(position);

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

inline bool compareStops(const QPair<qreal, QColor> &p1, const QPair<qreal, QColor> &p2)
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

qreal Gradient::extendPos(qreal position) const
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

qreal Gradient::interpolate(qreal v1, qreal v2, qreal amt)
{
	return v1 * (1.0-amt) + v2 * amt;
}

QColor Gradient::interpolate(const GradientStop& s1, const GradientStop& s2, qreal pos)
{
	qreal amt = (pos - s1.first) / (s2.first - s1.first);
	return interpolate(s1.second, s2.second, amt);
}

QColor Gradient::interpolate(const QColor& v1, const QColor& v2, qreal amt)
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
		qreal pos = stopNode.attribute("pos").toDouble();
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

} // namespace types

} // namespace panda
