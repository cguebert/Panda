#include <panda/helper/Point.h>

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

QPointF normalizeWithNorm(const QPointF& p, qreal norm, qreal threshold)
{
	if (norm > threshold)
		return p / norm;
	return p;
}

QPointF normalize(const QPointF &p, qreal threshold)
{
	return normalizeWithNorm(p, norm(p), threshold);
}

bool isNormalized(const QPointF& p, qreal threshold)
{
	return qAbs( norm2(p)-(qreal)1 ) <= threshold;
}

qreal sum(const QPointF& p)
{
	return p.x()+p.y();
}

} // namespace helper

} // namespace panda
