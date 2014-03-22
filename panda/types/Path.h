#ifndef TYPES_PATH_H
#define TYPES_PATH_H

#include <QVector>
#include <QPointF>

namespace panda
{

namespace types
{

class Path : public QVector<QPointF>
{
public:
	Path& operator=(const QVector<QPointF>& v);
};

} // namespace types

} // namespace panda

#endif // TYPES_PATH_H
