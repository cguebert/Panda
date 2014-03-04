#ifndef PATH_H
#define PATH_H

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

#endif // PATH_H
