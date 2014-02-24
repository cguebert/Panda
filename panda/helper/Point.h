#ifndef HELPER_POINT_H
#define HELPER_POINT_H

#include <QPointF>

namespace panda
{

namespace helper
{

qreal dot(const QPointF& p1, const QPointF& p2);

QPointF linearProduct(const QPointF& p1, const QPointF& p2);
QPointF linearDivision(const QPointF& p1, const QPointF& p2);

qreal norm2(const QPointF& p);
qreal norm(const QPointF& p);

/// Normalize the vector with the given norm
QPointF normalizeWithNorm(const QPointF& p, qreal norm, qreal threshold=std::numeric_limits<qreal>::epsilon());
QPointF normalize(const QPointF &p, qreal threshold=std::numeric_limits<qreal>::epsilon());

/// return true iff norm()==1
bool isNormalized(const QPointF& p, qreal threshold=std::numeric_limits<qreal>::epsilon()*(qreal)10 );

qreal sum(const QPointF& p);

} // namespace helper

} // namespace panda

#endif // HELPER_POINT_H
