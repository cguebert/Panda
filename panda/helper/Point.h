#ifndef HELPER_POINT_H
#define HELPER_POINT_H

#include <QPointF>
#include <qmath.h>

namespace panda
{

namespace helper
{

qreal dot(const QPointF& p1, const QPointF& p2)
{
	return p1.x()*p2.x() + p1.y()*p2.y();
}

QPointF linearProduct(const QPointF& p1, const QPointF& p2)
{
	return QPointF(p1.x()*p2.x(), p1.y()*p2.y());
}

QPointF linearDivision(const QPointF& p1, const QPointF& p2)
{
	return QPointF(p1.x()/p2.x(), p1.y()/p2.y());
}

qreal norm2(const QPointF& p)
{
	 return p.x()*p.x() + p.y()*p.y();
}

qreal norm(const QPointF& p)
{
	return qSqrt(norm2(p));
}

/// Normalize the vector with the given norm
QPointF normalizeWithNorm(const QPointF& p, qreal norm, qreal threshold=std::numeric_limits<qreal>::epsilon())
{
	if (norm > threshold)
		return p / norm;
	return p;
}

QPointF normalize(const QPointF &p, qreal threshold=std::numeric_limits<qreal>::epsilon())
{
	return normalizeWithNorm(p, norm(p), threshold);
}

/// return true iff norm()==1
bool isNormalized(const QPointF& p, qreal threshold=std::numeric_limits<qreal>::epsilon()*(qreal)10 )
{
	return qAbs( norm2(p)-(qreal)1 ) <= threshold;
}

qreal sum(const QPointF& p)
{
	return p.x()+p.y();
}

} // namespace helper

} // namespace panda

#endif // HELPER_POINT_H
